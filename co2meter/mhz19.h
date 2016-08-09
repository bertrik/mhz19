#include <stdbool.h>

bool process_rx(uint8_t b, uint8_t cmd, uint8_t data[]);
int prepare_tx(uint8_t cmd, const uint8_t *data, uint8_t buffer[], int size);

