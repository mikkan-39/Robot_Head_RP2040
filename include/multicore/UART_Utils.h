#ifndef _UART_UTILS_
#define _UART_UTILS_

#include "MulticoreUtils.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define START_BYTE 0xAA

uint8_t calculate_checksum(uint8_t *data, int length);
bool    validate_checksum(uint8_t *data, int length);
void    send_status(uint8_t status);

#endif
