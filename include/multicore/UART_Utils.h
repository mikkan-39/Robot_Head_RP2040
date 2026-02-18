#ifndef _UART_UTILS_
#define _UART_UTILS_

#include "MulticoreUtils.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <cstdio>

#define START_BYTE 0xAA

void send_tof_via_uart(uint16_t value);
void send_imu_via_uart(float *data, int length);

uint8_t calculate_checksum(uint8_t *data, int length);
bool validate_checksum(uint8_t *data, int length);
void send_status(uint8_t status);

#endif