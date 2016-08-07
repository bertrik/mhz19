#include <stdint.h>
#include <stdbool.h>

#define TIMEOUT 1000

typedef enum {
    START_BYTE,
    COMMAND,
    DATA,
    CHECK
} state_t;


bool process_byte(uint8_t b, uint8_t cmd, uint8_t data[])
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
            process_byte(b, cmd, data);
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

