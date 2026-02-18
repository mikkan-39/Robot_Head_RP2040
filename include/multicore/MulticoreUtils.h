#ifndef _MULTICORE_UTILS_
#define _MULTICORE_UTILS_

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

extern queue_t command_queue;

auto_init_mutex(eyeSettingMutex);

void send_char_to_core0(char c);
char read_char_from_core1();

enum MainCommands : uint8_t {
  PING         = 0x01,
  DRAW_INIT    = 0x02,
  DRAW_LOADING = 0x03,
  DRAW_ERROR   = 0x04,
  DRAW_EYES    = 0x05,
};

enum StatusCodes : uint8_t {
  OK = 0x00,
  INVALID_COMMAND = 0x01,
  INVALID_ARG = 0x02,
  WRONG_CHECKSUM = 0x03,
  INCOMPLETE_REQUEST = 0x04
};

typedef struct {
  int x;                    // x[int]
  int y;                    // y[int]
  int radius;               // r[int]
  int speed;                // s[int]
  uint16_t backgroundColor; // b[uint16_t]
  uint16_t primaryColor;    // p[uint16_t]
  uint16_t secondaryColor;  // c[uint16_t] (s for speed)
  uint16_t reserveColor;    // u[uint16_t] (r for radius)
} EyeSettings;

extern EyeSettings desired_settings;

#endif
