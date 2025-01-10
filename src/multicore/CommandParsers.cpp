#include "multicore/CommandParsers.h"

void parse_command(const char *command,
                   void (*ping_handler)(),
                   void (*imu_handler)(),
                   void (*tof_handler)(),
                   void (*draw_parser)(const char *),
                   void (*unknown_handler)(const char *)) {
  if (strcmp(command, "PING") == 0) {
    ping_handler();
  } else if (strcmp(command, "READ_IMU") == 0) {
    imu_handler();
  } else if (strcmp(command, "READ_TOF") == 0) {
    tof_handler();
  } else if (strncmp(command, "DRAW", 4) == 0) {
    draw_parser(command);
  } else {
    // Command not recognized
    unknown_handler(command);
  }
}

void parse_draw_command(
    const char *command, EyeSettings *eye_settings,
    void (*init_handler)(), void (*error_handler)(),
    void (*loading_handler)(), void (*eyes_handler)(),
    void (*unknown_handler)(const char *)) {
  if (strcmp(command, "DRAW_INIT") == 0) {
    init_handler();
  } else if (strcmp(command, "DRAW_ERROR") == 0) {
    error_handler();
  } else if (strcmp(command, "DRAW_LOADING") == 0) {
    loading_handler();
  } else if (strncmp(command, "DRAW_EYES", 9) == 0) {
    // Update EyeSettings
    const char *args = command + 9; // Skip "DRAW_EYES"
    while (*args != '\0') {
      if (*args == ' ') {
        args++; // Skip spaces
        continue;
      }
      char key = *args++;
      if (*args == '\0')
        break; // End of string

      int value = strtol(args, (char **)&args, 10);

      if (key == 'x') // x coord
        eye_settings->x = value;

      else if (key == 'y') // y coord
        eye_settings->y = value;

      else if (key == 'r') // radius
        eye_settings->radius = value;

      else if (key == 's') // speed
        eye_settings->speed = value;

      else if (key == 'b') // backgroundColor
        eye_settings->backgroundColor = (uint16_t)value;

      else if (key == 'p') // primaryColor
        eye_settings->primaryColor = (uint16_t)value;

      else if (key == 'c') // secondaryColor
        eye_settings->secondaryColor = (uint16_t)value;

      else if (key == 'u') // reserveColor
        eye_settings->reserveColor = (uint16_t)value;
    }
    eyes_handler();
  } else {
    // Command not recognized
    unknown_handler(command);
  }
}