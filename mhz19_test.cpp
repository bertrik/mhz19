// simple test code to verify creation of tx command and parsing of rx response 

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "co2meter/mhz19.cpp"

static bool test_rx(void)
{
   uint8_t data[10];

    // send some garbage bytes
    process_rx(0x00, 0x86, data);
    process_rx(0xFF, 0x86, data);

    // send the actual frame
    process_rx(0xFF, 0x86, data);
    process_rx(0x86, 0x86, data);
    process_rx(0x02, 0x86, data);
    process_rx(0x60, 0x86, data);
    process_rx(0x47, 0x86, data);
    process_rx(0x00, 0x86, data);
    process_rx(0x00, 0x86, data);
    process_rx(0x00, 0x86, data);
    bool b = process_rx(0xD1, 0x86, data);

    return b;    
}

static bool test_tx(void)
{
    int res;

    // verify empty buffer
    res = prepare_tx(0, NULL, NULL, 0);
    if (res > 0) {
        fprintf(stderr, "expected failure for too small buffer!");
        return false;
    }

    // verify valid message
    uint8_t buf[9];
    uint8_t data[] = {0, 0, 0, 0, 0};
    res = prepare_tx(0x86, data, buf, sizeof(buf));
    if (res != 9) {
        fprintf(stderr, "expected 9 bytes!");
        return false;
    }

    const uint8_t expected[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    return (memcmp(expected, buf, sizeof(expected)) == 0);
}


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    bool b;

    printf("test_rx ...");
    b = test_rx();
    printf("%s\n", b ? "PASS" : "FAIL");

    printf("test_tx ...");
    b = test_tx();
    printf("%s\n", b ? "PASS" : "FAIL");

    return 0;
}

