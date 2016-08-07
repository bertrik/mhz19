#include <stdio.h>
#include "co2meter/mhz19.cpp"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    uint8_t data[10];

    // send some garbage bytes
    process_byte(0x00, 0x86, data);
    process_byte(0xFF, 0x86, data);

    // send the actual frame
    process_byte(0xFF, 0x86, data);
    process_byte(0x86, 0x86, data);
    process_byte(0x02, 0x86, data);
    process_byte(0x60, 0x86, data);
    process_byte(0x47, 0x86, data);
    process_byte(0x00, 0x86, data);
    process_byte(0x00, 0x86, data);
    process_byte(0x00, 0x86, data);
    bool b = process_byte(0xD1, 0x86, data);

    printf("result: %s\n", b ? "PASS" : "FAIL");

    int res = b ? 0 : -1;
    return res;
}

