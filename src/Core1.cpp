#include "Core1.h"

// Madgwick filter;

// Adafruit_NXPSensorFusion filter3; // slowest
Adafruit_Madgwick filter; // faster than NXP
Adafruit_Mahony filter2;  // fastest/smalleset

float gx, gy, gz, ax, ay, az, mx, my, mz, vx, vy, vz, qx,
    qy, qz, qw;
float yaw, pitch, roll;

uint16_t TOFDistance = 0;

float sampleRate = 500;

void IMU_handler() {
  float imudata[] = {qx, qy, qz, qw, vx, vy, vz,
                     ax, ay, az, gx, gy, gz};

  send_imu_via_uart(imudata, 13);
}

void TOF_handler() { send_tof_via_uart(TOFDistance); }

void handle_draw_eyes_command(const uint8_t *payload,
                              uint8_t length) {
  size_t i = 0;
  while (i + 2 <= length) {
    uint8_t tag = payload[i++];
    uint8_t field_len = payload[i++];

    if (i + field_len > length)
      break;

    switch (tag) {
    case 0x01: // x
      if (field_len == 1)
        desired_settings.x = *(uint8_t *)(payload + i);
      break;
    case 0x02: // y
      if (field_len == 1)
        desired_settings.y = *(uint8_t *)(payload + i);
      break;
    case 0x03: // radius
      if (field_len == 1)
        desired_settings.radius = *(uint8_t *)(payload + i);
      break;
    case 0x04: // speed
      if (field_len == 1)
        desired_settings.speed = *(uint8_t *)(payload + i);
      break;
    case 0x05: // backgroundColor
      if (field_len == 2)
        desired_settings.backgroundColor =
            *(uint16_t *)(payload + i);
      break;
    case 0x06: // primaryColor
      if (field_len == 2)
        desired_settings.primaryColor =
            *(uint16_t *)(payload + i);
      break;
    case 0x07: // secondaryColor
      if (field_len == 2)
        desired_settings.secondaryColor =
            *(uint16_t *)(payload + i);
      break;
    case 0x08: // reserveColor
      if (field_len == 2)
        desired_settings.reserveColor =
            *(uint16_t *)(payload + i);
      break;
    default:
      break; // unknown tag — skip or log
    }

    i += field_len;
  }
}

void parse_command(uint8_t command, uint8_t length,
                   const uint8_t *payload) {

  switch (command) {
  case MainCommands::PING:
    send_status(StatusCodes::OK);
    break;
  case MainCommands::TOF:
    TOF_handler();
    break;
  case MainCommands::IMU:
    IMU_handler();
    break;

  case MainCommands::DRAW_INIT:
  case MainCommands::DRAW_LOADING:
  case MainCommands::DRAW_ERROR:
    send_char_to_core0(command);
    send_status(StatusCodes::OK);
    break;

    //[START_BYTE] [0x07 (DRAW_EYES)] [PayloadLength]
    //[tag (1 byte) length (1 byte) data (length bytes)]
    //[...repeat...]
    //[Checksum]
  case MainCommands::DRAW_EYES:
    send_status(StatusCodes::OK);
    mutex_enter_blocking(&eyeSettingMutex);
    handle_draw_eyes_command(payload, length);
    send_char_to_core0(MainCommands::DRAW_EYES);
    mutex_exit(&eyeSettingMutex);
    break;

  default:
    send_status(StatusCodes::INVALID_COMMAND);
    break;
  }
}

static bool syncing = true;
void on_serial_rx(uint8_t byte, bool timeout) {
  static uint8_t buffer[256];
  static uint8_t pos = 0;
  static uint8_t expected_len = 0;

  if (syncing) {
    if (byte == START_BYTE) {
      pos = 0;
      buffer[pos++] = byte;
      syncing = false;
    }
    return;
  }

  buffer[pos++] = byte;

  if (pos == 2) {
    // Received PACKET_TYPE
  } else if (pos == 3) {
    // Received PACKET_LENGTH
    expected_len = buffer[2];
  } else if (pos == 3 + expected_len + 1) {
    // Full packet + checksum received
    if (validate_checksum(buffer, pos)) {
      // if (true) {
      parse_command(buffer[1], buffer[2], buffer + 3);
    } else {
      send_status(StatusCodes::WRONG_CHECKSUM);
    }
    syncing = true;
  }

  if (pos >= sizeof(buffer)) {
    syncing = true; // Avoid overflow
  }
}

bool IMU_timer_callback(repeating_timer_t *rt) {
  accelerometer.readAccelerationGXYZ(ax, ay, az);
  gyroscope.readRotationDegXYZ(gx, gy, gz);
  compass.readCalibrateMagneticGaussXYZ(mx, my, mz);

  filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

  filter.getGravityVector(&vx, &vy, &vz);
  filter.getQuaternion(&qw, &qx, &qy, &qz);

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
    if (uart_is_readable_within_us(uart1, 1000)) {
      char uart_data = uart_getc(uart1);
      on_serial_rx(uart_data, false);
    } else {
      if (!syncing) {
        syncing = true;
        send_status(StatusCodes::INCOMPLETE_REQUEST);
      }
    }
  }
}
