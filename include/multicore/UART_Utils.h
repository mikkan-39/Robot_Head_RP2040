#ifndef _UART_UTILS_
#define _UART_UTILS_

#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <cstdio>

void send_string_via_uart(const char *string);
void send_uint16_via_uart(uint16_t value);
void send_float_via_uart(float value);
void send_newline_via_uart();
char *receive_command_from_uart();

#endif