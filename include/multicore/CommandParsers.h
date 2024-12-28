
#ifndef _COMMAND_PARSERS_
#define _COMMAND_PARSERS_

#include <cstdint>
#include <cstdlib>
#include <cstring>

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

void parse_command(const char *command,
                   void (*imu_handler)(),
                   void (*tof_handler)(),
                   void (*draw_parser)(const char *),
                   void (*unknown_handler)(const char *));

void parse_draw_command(
    const char *command, EyeSettings *eye_settings,
    void (*error_handler)(), void (*loading_handler)(),
    void (*eyes_handler)(),
    void (*unknown_handler)(const char *));

#endif