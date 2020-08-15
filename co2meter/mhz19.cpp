#include "Arduino.h"

#include <stdint.h>
#include <stdbool.h>

#include <Stream.h>

#include "mhz19.h"

#define CMD_SIZE 9
#define TIMEOUT_MS  1000

MHZ19::MHZ19(Stream *serial)
{
    _serial = serial;
    _state = START_BYTE;
    _check = 0;
    _idx = 0;
    _len = 0;
}

int MHZ19::sendCommand(uint8_t cmd_data[], uint8_t rsp_data[], unsigned int timeout_ms)
{
    uint8_t cmdbuf[9];

    // build command
    int idx = 0;
    cmdbuf[idx++] = 0xFF;
    cmdbuf[idx++] = 0x01;
    uint8_t check = cmdbuf[0] + cmdbuf[1];
    for (int i = 0; i < 6; i++) {
        cmdbuf[idx++] = cmd_data[i];
        check += cmd_data[i];
    }
    cmdbuf[idx++] = 255 - check;

    // send the command
    _serial->write(cmdbuf, idx++);

    // wait for response
    unsigned long start = millis();
    while ((millis() - start) < timeout_ms) {
        if (_serial->available() > 0) {
            uint8_t b = _serial->read();
            if (process_rx(b, cmd_data[0])) {
                memcpy(rsp_data, _data, _len);
                return _len;
            }
        }
        yield();
    }

    return 0;
}

bool MHZ19::readCo2(int *co2, int *temp)
{
    uint8_t cmd_data[] = { 0x86, 0, 0, 0, 0, 0 };
    uint8_t rsp_data[6];

    int rsp_len = sendCommand(cmd_data, rsp_data, TIMEOUT_MS);
    if (rsp_len > 2) {
        *co2 = (rsp_data[0] << 8) + rsp_data[1];
        *temp = rsp_data[2] - 40;
        return true;
    }
    return false;
}

/**
    Processes one received byte.
    @param b the byte
    @param cmd the command code
    @return true if a full message was received, false otherwise
 */
bool MHZ19::process_rx(uint8_t b, uint8_t cmd)
{
    // update checksum
    _check += b;

    switch (_state) {
    case START_BYTE:
        if (b == 0xFF) {
            _check = 0;
            _state = COMMAND;
        }
        break;
    case COMMAND:
        if (b == cmd) {
            _idx = 0;
            _len = 6;
            _state = DATA;
        } else {
            _state = START_BYTE;
            process_rx(b, cmd);
        }
        break;
    case DATA:
        _data[_idx++] = b;
        if (_idx == _len) {
            _state = CHECK;
        }
        break;
    case CHECK:
        _state = START_BYTE;
        return (_check == 0);
    default:
        _state = START_BYTE;
        break;
    }

    return false;
}

