#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <math.h>
#include <pico/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "GUI_Paint.h"
#include "drivers/GpioUtils.h"
#include "drivers/LCD_Driver.h"

#include "lcd.pio.h"

int restrainedCoords(int coords) { return std::min(std::max(coords, 0), 239); }

int main() {
  stdio_init_all();

  InitAllGpio();

  Bitmap_Init(&BitmapRight, CYAN, MAGENTA, BLACK);
  Bitmap_Init(&BitmapLeft, MAGENTA, CYAN, BLACK);

  LCD_Both_Init();

  while (true) {
  }
}
