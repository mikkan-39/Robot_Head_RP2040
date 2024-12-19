#include "drivers/TOF_Driver.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <cstdio>

void core1_thread() {
  //   printf("Core 1: Starting...\n");

  while (true) {
    // Read data from UART
    if (uart_is_readable(uart0)) {
      uint8_t uart_data = uart_getc(uart0);

      // Forward data to Core 0
      multicore_fifo_push_blocking(uart_data);
    }

    printf("%d", TOFsensor.readRangeSingleMillimeters());
    if (TOFsensor.timeoutOccurred()) {
      printf(" TIMEOUT");
    }
    printf("\n");
    busy_wait_ms(1);
  }
}
