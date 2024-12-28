#include "multicore/UART_Utils.h"

void send_string_via_uart(const char *string) {
  uart_puts(uart0, string);
}

void send_uint16_via_uart(uint16_t value) {
  char buffer[8];               // Buffer to hold the string
                                // representation
  sprintf(buffer, "%d", value); // Convert integer to string
  uart_puts(uart0, buffer);     // Send the string via UART
}

void send_float_via_uart(float value) {
  char buffer[16]; // Buffer to hold the string
                   // representation of the float
  sprintf(buffer, "%.8f", value); // Convert float to string
                                  // with 2 decimal places
  uart_puts(uart0, buffer); // Send the string via UART
}

void send_newline_via_uart() { uart_puts(uart0, "\n"); }

char command[128];
char *receive_command_from_uart() {

  int i = 0;

  while (i < 127) { // Reserve 1 byte for the null
                    // terminator
    if (uart_is_readable(uart0)) {
      uint8_t byte = uart_getc(uart0);

      if (byte == '\n' || byte == '\r') {
        break;
      }

      command[i] = byte;
      i++;
    }
  }

  command[i] = '\0';

  return command;
}