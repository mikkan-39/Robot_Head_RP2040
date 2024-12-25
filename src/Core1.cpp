#include "Core1.h"

Madgwick filter;

float gx, gy, gz, ax, ay, az;
float yaw, pitch, roll;

uint16_t TOFDistance = 0;

uint64_t previousEndTime;

float sampleRate = 100;

void IMU_handler() {
  send_string_via_uart("IMU: ");
  send_float_via_uart(yaw);
  send_string_via_uart(" ");
  send_float_via_uart(pitch);
  send_string_via_uart(" ");
  send_float_via_uart(roll);
  // send_newline_via_uart();
  send_string_via_uart(" ACC ");
  send_float_via_uart(ax);
  send_string_via_uart(" ");
  send_float_via_uart(ay);
  send_string_via_uart(" ");
  send_float_via_uart(az);
  send_string_via_uart(" GYRO ");
  send_float_via_uart(gx);
  send_string_via_uart(" ");
  send_float_via_uart(gy);
  send_string_via_uart(" ");
  send_float_via_uart(gz);
  send_newline_via_uart();
}
void TOF_handler() {
  send_string_via_uart("TOF: ");
  send_uint16_via_uart(TOFDistance);
  send_newline_via_uart();
}
void unknown_handler_main(const char *cmd) {
  send_string_via_uart("UNKNOWN COMMAND: ");
  send_string_via_uart(cmd);
  send_newline_via_uart();
}
#define draw_handler send_string_to_core0

bool IMU_timer_callback(repeating_timer_t *rt) {
  filter.setFrequency(sampleRate);
  accelerometer.readAccelerationGXYZ(ax, ay, az);
  gyroscope.readRotationRadXYZ(gx, gy, gz);

  filter.update(gx, gy, gz, ax, ay, az);

  yaw = filter.getYawDeg();
  pitch = filter.getPitchDeg();
  roll = filter.getRollDeg();

  uint64_t deltaTime = time_us_64() - previousEndTime;
  sampleRate = 1000000.0f / (float)deltaTime;
  previousEndTime = time_us_64();

  return true;
}

bool TOF_timer_callback(repeating_timer_t *rt) {
  uint16_t nextTOFDistance =
      TOFsensor.readRangeSingleMillimeters();
  if (!TOFsensor.timeoutOccurred()) {
    TOFDistance = nextTOFDistance;
  }
  return true;
}

void core1_thread() {
  filter.begin();

  struct repeating_timer IMUtimer;
  struct repeating_timer TOFtimer;

  previousEndTime = time_us_64();
  // every 10 ms
  add_repeating_timer_ms(100, IMU_timer_callback, NULL,
                         &IMUtimer);
  add_repeating_timer_ms(200, TOF_timer_callback, NULL,
                         &TOFtimer);

  while (true) {
    if (uart_is_readable(uart0)) {
      char *uart_data = receive_command_from_uart();
      parse_command(uart_data, IMU_handler, TOF_handler,
                    draw_handler, unknown_handler_main);
    }
  }
}
