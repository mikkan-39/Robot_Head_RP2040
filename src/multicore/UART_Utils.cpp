#include "multicore/UART_Utils.h"

char command[128];
char *receive_command_from_uart() {

  int i = 0;

  while (i < 127) { // Reserve 1 byte for the null
                    // terminator
    if (uart_is_readable(uart1)) {
      uint8_t byte = uart_getc(uart1);

      if (byte == '\n' || byte == '\r') {
        break;
      }

      command[i] = byte;
      i++;
    }
  }

  command[i] = '\0';

  return command;
}

uint8_t calculate_checksum(uint8_t *data, int length) {
  char crc = 0x00;
  char extract;
  char sum;
  for (int i = 0; i < length; i++) {
    extract = *data;
    for (char tempI = 8; tempI; tempI--) {
      sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum)
        crc ^= 0x8C;
      extract >>= 1;
    }
    data++;
  }
  return crc;
}

bool validate_checksum(uint8_t *data, int length) {
  uint8_t crc = calculate_checksum(data, length - 1);
  return crc == data[length - 1];
}

void send_status(uint8_t status) {
  uint8_t packet[] = {START_BYTE, status, 0x00, 0x00};
  packet[3] = calculate_checksum(packet, 3);
  uart_write_blocking(uart1, packet, 4);
}

void send_tof_via_uart(uint16_t value) {
  uint8_t packet[] = {
      START_BYTE, StatusCodes::OK, 0x02, 0x00, 0x00, 0x00};
  packet[3] = value & 0xFF;        // LSB
  packet[4] = (value >> 8) & 0xFF; // MSB
  packet[5] = calculate_checksum(packet, 5);
  uart_write_blocking(uart1, packet, 6);
}

//[START_BYTE] [StatusCode=OK] [PayloadLength=num_floats*4]
//[Data...num_floats*4 bytes] [Checksum]
void send_imu_via_uart(float *data, int length) {
  const uint8_t payload_length = length * 4;
  uint8_t packet[3 + payload_length + 1]; // header +
                                          // payload +
                                          // checksum

  packet[0] = START_BYTE;
  packet[1] = StatusCodes::OK;
  packet[2] = payload_length;

  // Write floats into packet
  uint8_t *payload_ptr = &packet[3];

  for (int i = 0; i < length; ++i) {
    uint8_t *fbytes = (uint8_t *)&data[i];
    for (int j = 0; j < 4; ++j) {
      payload_ptr[i * 4 + j] = fbytes[j]; // copy float as
                                          // bytes (LE)
    }
  }

  // Checksum over header + payload
  packet[3 + payload_length] =
      calculate_checksum(packet, 3 + payload_length);

  uart_write_blocking(uart1, packet, sizeof(packet));
}