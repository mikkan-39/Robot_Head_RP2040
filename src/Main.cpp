#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <math.h>
#include <pico/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "CommandParsers.h"
#include "Core1.h"
#include "MulticoreUtils.h"
#include "drivers/GpioUtils.h"
#include "gui/GUI_Paint.h"

#include "lcd.pio.h"

// Global instance of EyeSettings
EyeSettings eye_settings = {120,  120,   90,     BLACK,
                            CYAN, GREEN, MAGENTA};

void error_handler() { printf("DRAW ERROR ACK\n"); }
void loading_handler() { printf("DRAW LOADING ACK\n"); }
void eyes_handler() { printf("DRAW EYES ACK\n"); }
void unknown_handler_draw(const char *cmd) {
  printf("UNKNOWN COMMAND: ");
  printf(cmd);
  printf("\n");
}

int restrainedCoords(int coords) {
  return std::min(std::max(coords, 0), 239);
}

int main() {
  stdio_init_all();

  InitAllGpio();

  Bitmap_Init(&BitmapRight, CYAN, MAGENTA, BLACK);
  Bitmap_Init(&BitmapLeft, CYAN, MAGENTA, BLACK);

  LCD_Both_Init();

  BitmapsSend();

  queue_init_with_spinlock(&command_queue, MAX_COMMAND_SIZE,
                           4, 0);

  multicore_launch_core1(core1_thread);

  while (true) {
    if (!queue_is_empty(&command_queue)) {
      char *cmd = receive_string_from_core1();
      printf("Command received by core0\n");
      parse_draw_command(cmd, &eye_settings, error_handler,
                         loading_handler, eyes_handler,
                         unknown_handler_draw);
    }

    BitmapRight.PrimaryColor = eye_settings.primaryColor;
    BitmapLeft.PrimaryColor = eye_settings.primaryColor;

    BitmapRight.UpdateColorLookup();
    BitmapLeft.UpdateColorLookup();

    DrawEye(eye_settings.x, eye_settings.y,
            eye_settings.radius);
  }
}
