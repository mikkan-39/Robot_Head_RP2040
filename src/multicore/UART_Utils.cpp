#include "multicore/UART_Utils.h"

uint8_t calculate_checksum(uint8_t *data, int length) {
  uint8_t crc = 0x00;
  for (int i = 0; i < length; i++) {
    uint8_t extract = data[i];
    for (int b = 8; b; b--) {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) crc ^= 0x8C;
      extract >>= 1;
    }
  }
  return crc;
}

bool validate_checksum(uint8_t *data, int length) {
  return calculate_checksum(data, length - 1) == data[length - 1];
}

void send_status(uint8_t status) {
  uint8_t packet[] = {START_BYTE, status, 0x00, 0x00};
  packet[3] = calculate_checksum(packet, 3);
  uart_write_blocking(uart1, packet, 4);
}
