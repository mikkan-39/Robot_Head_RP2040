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
#include "drivers/GpioUtils.h"
#include "gui/GUI_Paint.h"
#include "multicore/CommandParsers.h"
#include "multicore/MulticoreUtils.h"

#include "lcd.pio.h"

EyeSettings desired_settings = {120,   120,  90,    1,
                                BLACK, CYAN, GREEN, RED};
EyeSettings current_settings = {120,   120,  1,     1,
                                BLACK, CYAN, GREEN, RED};

enum class SystemStates { INIT = 1, NORMAL = 2, ERROR = 3 };
SystemStates currentSystemState = SystemStates::INIT;

void bumpCurrentToDesired(EyeSettings *current,
                          EyeSettings *desired) {
  // set directly without transition
  current->speed = desired->speed;
  current->primaryColor = desired->primaryColor;
  current->secondaryColor = desired->secondaryColor;
  current->backgroundColor = desired->backgroundColor;
  current->reserveColor = desired->reserveColor;

  // Step size determines how much to move per iteration
  const int step = desired->speed;

  int dx = desired->x - current->x;
  int dy = desired->y - current->y;
  int dr = desired->radius - current->radius;

  int max_diff = abs(dx);
  if (abs(dy) > max_diff) {
    max_diff = abs(dy);
  }
  if (abs(dr) > max_diff) {
    max_diff = abs(dr);
  }

  if (max_diff == 0) {
    return;
  }

  float scale_x = (float)dx / max_diff;
  float scale_y = (float)dy / max_diff;
  float scale_r = (float)dr / max_diff;

  current->x += (int)(step * scale_x);
  current->y += (int)(step * scale_y);
  current->radius += (int)(step * scale_r);

  // Prevent overshooting
  if ((dx > 0 && current->x > desired->x) ||
      (dx < 0 && current->x < desired->x)) {
    current->x = desired->x;
  }
  if ((dy > 0 && current->y > desired->y) ||
      (dy < 0 && current->y < desired->y)) {
    current->y = desired->y;
  }
  if ((dr > 0 && current->radius > desired->radius) ||
      (dr < 0 && current->radius < desired->radius)) {
    current->radius = desired->radius;
  }
}

void updateBitmapColors() {
  BitmapRight.PrimaryColor = current_settings.primaryColor;
  BitmapLeft.PrimaryColor = current_settings.primaryColor;
  BitmapRight.BackgroundColor =
      current_settings.backgroundColor;
  BitmapLeft.BackgroundColor =
      current_settings.backgroundColor;
  BitmapRight.SecondaryColor =
      current_settings.secondaryColor;
  BitmapLeft.SecondaryColor =
      current_settings.secondaryColor;
  BitmapRight.ReservedColor = current_settings.reserveColor;
  BitmapLeft.ReservedColor = current_settings.reserveColor;
  BitmapRight.UpdateColorLookup();
  BitmapLeft.UpdateColorLookup();
}

void error_handler() {
  printf("DRAW ERROR ACK\n");
  DrawError();
  currentSystemState = SystemStates::ERROR;
}

void loading_handler() {
  printf("DRAW LOADING ACK\n");
  updateBitmapColors();
  DrawLoadingBlocking(
      currentSystemState != SystemStates::INIT,
      current_settings.x, current_settings.y,
      current_settings.radius);
  printf("LOADING ANIMATION FINISHED\n");
  currentSystemState = SystemStates::NORMAL;
}

void eyes_handler() {
  printf("DRAW EYES ACK\n");
  SystemStates::NORMAL;
}

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

  Bitmap_Init(&BitmapRight, current_settings.primaryColor,
              current_settings.secondaryColor,
              current_settings.backgroundColor,
              current_settings.reserveColor);
  Bitmap_Init(&BitmapLeft, current_settings.primaryColor,
              current_settings.secondaryColor,
              current_settings.backgroundColor,
              current_settings.reserveColor);

  LCD_Both_Init();

  // while (true) {
  //   current_settings.backgroundColor = CYAN;
  //   updateBitmapColors();
  //   BitmapsSend();
  //   current_settings.backgroundColor = MAGENTA;
  //   updateBitmapColors();
  //   BitmapsSend();
  // };

  BitmapsSend();

  queue_init_with_spinlock(&command_queue, MAX_COMMAND_SIZE,
                           4, 0);

  multicore_launch_core1(core1_thread);

  uint16_t nextColor;

  while (true) {
    if (!queue_is_empty(&command_queue)) {
      char *cmd = receive_string_from_core1();
      printf("Command received by core0\n");
      parse_draw_command(cmd, &desired_settings,
                         error_handler, loading_handler,
                         eyes_handler,
                         unknown_handler_draw);
    }

    if (currentSystemState == SystemStates::INIT) {
      DrawInit();
      BitmapsSend();
    }

    if (currentSystemState == SystemStates::ERROR) {
      nextColor =
          getPulsingColor(desired_settings.reserveColor);
      current_settings.reserveColor = nextColor;
      updateBitmapColors();
      BitmapsSend();
    }

    if (currentSystemState == SystemStates::NORMAL) {
      BitmapsClear();
      bumpCurrentToDesired(&current_settings,
                           &desired_settings);
      updateBitmapColors();
      DrawEye(current_settings.x, current_settings.y,
              current_settings.radius, PRIMARY_COLOR);
      BitmapsSend();
    }
  }
}
