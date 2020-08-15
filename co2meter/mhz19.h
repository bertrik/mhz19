#include <stdbool.h>

typedef enum {
    START_BYTE,
    COMMAND,
    DATA,
    CHECK
} state_t;

class MHZ19 {

private:
    Stream *_serial;
    state_t _state;
    uint8_t _check;
    int _idx;
    int _len;
    uint8_t _data[20];

    bool process_rx(uint8_t b, uint8_t cmd);

public:
    static const int BIT_RATE = 9600;
    static const int RSP_SIZE = 6;

    /**
     * Constructor.
     *
     * @param serial the serial port, NOTE: the serial port has to be configured for a bit rate of MHZ19::BIT_RATE !
     */
    explicit MHZ19(Stream *serial);

    /**
     * Reads CO2 level (and temperature).
     *
     * @param co2 the CO2 level, in ppm
     * @param temperature the temperature, in degrees Celcius
     * @return true if the CO2 level was read successfully, false otherwise
     */
    bool readCO2(int *co2, int *temp);

    /**
     * Sends a custom command to the sensor, waits for and returns the response.
     *
     * @param cmd_data command data (e.g. [0x86, 0, 0, 0, 0, 0])
     * @param rsp_data a buffer for the response data (at least MHZ19::RSP_SIZE bytes)
     * @param timeout_ms the maximum time (ms) to wait for a response
     * @return the length of the response, <= 0 indicates failure
     */
    int sendCommand(const uint8_t cmd_data[], uint8_t rsp_data[], unsigned int timeout_ms);
};

