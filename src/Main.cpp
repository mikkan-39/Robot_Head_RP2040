#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <math.h>
#include <pico/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Core1.h"
#include "MulticoreUtils.h"
#include "drivers/GpioUtils.h"
#include "gui/GUI_Paint.h"

#include "lcd.pio.h"

int restrainedCoords(int coords) {
  return std::min(std::max(coords, 0), 239);
}

int main() {
  stdio_init_all();

  InitAllGpio();

  Bitmap_Init(&BitmapRight, CYAN, MAGENTA, BLACK);
  Bitmap_Init(&BitmapLeft, MAGENTA, CYAN, BLACK);

  LCD_Both_Init();

  queue_init_with_spinlock(&command_queue, MAX_COMMAND_SIZE,
                           4, 0);

  multicore_launch_core1(core1_thread);

  while (true) {
    if (!queue_is_empty(&command_queue)) {
      char *commandP =
          receive_string_from_core1(); // Receive
                                       // UART
                                       // data

      printf("Command received by core0\n");
      printf(commandP);
    }
  }
}
