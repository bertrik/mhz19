#include <stdint.h>
#include <stdbool.h>

#define CMD_SIZE 9 

typedef enum {
    START_BYTE,
    COMMAND,
    DATA,
    CHECK
} state_t;

/** 
    Prepares a command buffer to send to an mhz19.
    @param data tx data
    @param buffer the tx buffer to fill
    @param size the size of the tx buffer
    @return number of bytes in buffer
*/
int prepare_tx(uint8_t cmd, const uint8_t *data, uint8_t buffer[], int size)
{
    if (size < CMD_SIZE) {
        return 0;
    }

    // create command buffer
    buffer[0] = 0xFF;
    buffer[1] = 0x01;
    buffer[2] = cmd;
    for (int i = 3; i < 8; i++) {
        buffer[i] = *data++;
    }

    // calculate checksum
    uint8_t check = 0;
    for (int i = 0; i < 8; i++) {
        check += buffer[i];
    }
    buffer[8] = 255 - check;

    return CMD_SIZE;
}

/**
    Processes one received byte.
    @param b the byte
    @param cmd the command code
    @param data the buffer to contain a received message
    @return true if a full message was received, false otherwise
 */
bool process_rx(uint8_t b, uint8_t cmd, uint8_t data[])
{
    static uint8_t check = 0;
    static int idx = 0;
    static int len = 0;
    static state_t state = START_BYTE;

    // update checksum
    check += b;

    switch (state) {
    case START_BYTE:
        if (b == 0xFF) {
            check = 0;
            state = COMMAND;
        }
        break;
    case COMMAND:
        if (b == cmd) {
            idx = 0;
            len = 6;
            state = DATA;
        } else {
            state = START_BYTE;
            process_rx(b, cmd, data);
        }
        break;
    case DATA:
        data[idx++] = b;
        if (idx == len) {
            state = CHECK;
        }
        break;
    case CHECK:
        state = START_BYTE;
        return (check == 0);
    default:
        state = START_BYTE;
        break;
    }

    return false;
}

