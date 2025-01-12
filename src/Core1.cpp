#include "Core1.h"

// Madgwick filter;

// Adafruit_NXPSensorFusion filter3; // slowest
Adafruit_Madgwick filter; // faster than NXP
Adafruit_Mahony filter2;  // fastest/smalleset

float gx, gy, gz, ax, ay, az, mx, my, mz;
float yaw, pitch, roll;

uint16_t TOFDistance = 0;

float sampleRate = 500;

void IMU_handler() {
  send_string_via_uart("READ_IMU: {");
  send_string_via_uart("pitch: ");
  send_float_via_uart(pitch);
  send_string_via_uart(", roll: ");
  send_float_via_uart(roll);
  send_string_via_uart(", yaw: ");
  send_float_via_uart(yaw);
  // send_newline_via_uart();
  send_string_via_uart(", ax: ");
  send_float_via_uart(ax);
  send_string_via_uart(", ay: ");
  send_float_via_uart(ay);
  send_string_via_uart(", az: ");
  send_float_via_uart(az);
  send_string_via_uart(", gx: ");
  send_float_via_uart(gx);
  send_string_via_uart(", gy: ");
  send_float_via_uart(gy);
  send_string_via_uart(", gz: ");
  send_float_via_uart(gz);
  // send_string_via_uart(", mx: ");
  // send_float_via_uart(mx);
  // send_string_via_uart(", my: ");
  // send_float_via_uart(my);
  // send_string_via_uart(", mz: ");
  // send_float_via_uart(mz);
  send_string_via_uart(", sampleRate: ");
  send_float_via_uart(sampleRate);
  send_string_via_uart("}");
  send_newline_via_uart();
}
void TOF_handler() {
  send_string_via_uart("READ_TOF: {distance: ");
  send_uint16_via_uart(TOFDistance);
  send_string_via_uart("}");
  send_newline_via_uart();
}
void pingHandler() {
  send_string_via_uart("PING ACK");
  send_newline_via_uart();
}
void unknown_handler_main(const char *cmd) {
  send_string_via_uart("UNKNOWN COMMAND: ");
  send_string_via_uart(cmd);
  send_newline_via_uart();
}
#define draw_handler send_string_to_core0

bool IMU_timer_callback(repeating_timer_t *rt) {
  accelerometer.readAccelerationGXYZ(ax, ay, az);
  gyroscope.readRotationDegXYZ(gx, gy, gz);
  // compass.readCalibrateMagneticGaussXYZ(mx, my, mz);

  filter.updateIMU(-gx, -gy, -gz, -ax, -ay, -az);

  yaw = filter.getYaw();
  pitch = filter.getPitch();
  roll = filter.getRoll();

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
  filter.begin(sampleRate);

  struct repeating_timer IMUtimer;
  struct repeating_timer TOFtimer;

  alarm_pool_t *alarm_pool =
      alarm_pool_create_with_unused_hardware_alarm(4);

  alarm_pool_add_repeating_timer_us(
      alarm_pool,
      static_cast<int>(1000000 / sampleRate) * -1,
      IMU_timer_callback, NULL, &IMUtimer);
  alarm_pool_add_repeating_timer_us(alarm_pool, 200000,
                                    TOF_timer_callback,
                                    NULL, &TOFtimer);

  while (true) {
    if (uart_is_readable(uart0)) {
      char *uart_data = receive_command_from_uart();
      parse_command(uart_data, pingHandler, IMU_handler,
                    TOF_handler, draw_handler,
                    unknown_handler_main);
    }
  }
}
