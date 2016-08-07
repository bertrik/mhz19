#include "Arduino.h"

#include "mhz19.h"

static bool exchange_command(uint8_t sensor_no, uint8_t cmd,
                             uint8_t data[], int timeout)
{
    // create command buffer
    uint8_t buf[9];
    buf[0] = 0xFF;
    buf[1] = sensor_no;
    buf[2] = cmd;
    for (int i = 3; i < 8; i++) {
        buf[i] = *data++;
    }

    // calculate checksum
    uint8_t check = 0;
    for (int i = 0; i < 8; i++) {
        check += buf[i];
    }
    buf[8] = 255 - check;

    // send the command
    Serial.write(buf, sizeof(buf));

    // wait for response
    long start = millis();
    while ((millis() - start) < timeout) {
        if (Serial.available() > 0) {
            uint8_t b = Serial.read();
            if (process_byte(b, cmd, data)) {
                return true;
            }
        }
    }

    return false;
}


void setup()
{
}

void loop()
{
}


