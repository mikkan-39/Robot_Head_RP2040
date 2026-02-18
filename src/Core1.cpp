#include "Core1.h"

static void handle_draw_eyes_command(const uint8_t *payload, uint8_t length) {
  size_t i = 0;
  while (i + 2 <= length) {
    uint8_t tag       = payload[i++];
    uint8_t field_len = payload[i++];
    if (i + field_len > length) break;

    switch (tag) {
    case 0x01: if (field_len == 1) desired_settings.x               = *(uint8_t  *)(payload + i); break;
    case 0x02: if (field_len == 1) desired_settings.y               = *(uint8_t  *)(payload + i); break;
    case 0x03: if (field_len == 1) desired_settings.radius          = *(uint8_t  *)(payload + i); break;
    case 0x04: if (field_len == 1) desired_settings.speed           = *(uint8_t  *)(payload + i); break;
    case 0x05: if (field_len == 2) desired_settings.backgroundColor = *(uint16_t *)(payload + i); break;
    case 0x06: if (field_len == 2) desired_settings.primaryColor    = *(uint16_t *)(payload + i); break;
    case 0x07: if (field_len == 2) desired_settings.secondaryColor  = *(uint16_t *)(payload + i); break;
    case 0x08: if (field_len == 2) desired_settings.reserveColor    = *(uint16_t *)(payload + i); break;
    default: break;
    }
    i += field_len;
  }
}

static void parse_command(uint8_t command, uint8_t length, const uint8_t *payload) {
  switch (command) {
  case MainCommands::PING:
    send_status(StatusCodes::OK);
    break;

  case MainCommands::DRAW_INIT:
  case MainCommands::DRAW_LOADING:
  case MainCommands::DRAW_ERROR:
    send_char_to_core0(command);
    send_status(StatusCodes::OK);
    break;

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

static void on_serial_rx(uint8_t byte) {
  static uint8_t buffer[256];
  static uint8_t pos          = 0;
  static uint8_t expected_len = 0;

  if (syncing) {
    if (byte == START_BYTE) {
      pos     = 0;
      buffer[pos++] = byte;
      syncing = false;
    }
    return;
  }

  buffer[pos++] = byte;

  if (pos == 3) {
    expected_len = buffer[2];
  } else if (pos == 3 + expected_len + 1) {
    if (validate_checksum(buffer, pos))
      parse_command(buffer[1], buffer[2], buffer + 3);
    else
      send_status(StatusCodes::WRONG_CHECKSUM);
    syncing = true;
  }

  if (pos >= sizeof(buffer)) {
    syncing = true;
  }
}

void core1_thread() {
  while (true) {
    if (uart_is_readable_within_us(uart1, 1000)) {
      on_serial_rx(uart_getc(uart1));
    } else {
      if (!syncing) {
        syncing = true;
        send_status(StatusCodes::INCOMPLETE_REQUEST);
      }
    }
  }
}
