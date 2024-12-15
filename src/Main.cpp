#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <math.h>
#include <pico/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Core1.h"
#include "drivers/GpioUtils.h"
#include "drivers/LCD_Driver.h"
#include "gui/GUI_Paint.h"

#include "lcd.pio.h"

int restrainedCoords(int coords) { return std::min(std::max(coords, 0), 239); }

int main() {
  stdio_init_all();

  InitAllGpio();

  Bitmap_Init(&BitmapRight, CYAN, MAGENTA, BLACK);
  Bitmap_Init(&BitmapLeft, MAGENTA, CYAN, BLACK);

  LCD_Both_Init();

  multicore_launch_core1(core1_thread);

  while (true) {
    if (multicore_fifo_rvalid()) {
      uint32_t data = multicore_fifo_pop_blocking(); // Receive UART data
    }
  }
}
