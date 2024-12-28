#include "drivers/IMU/BaseIMU.h"

uint8_t BaseIMU::readDeviceID() {
  return _readByte(BASE_IMU_WHO_AM_I);
}

int16_t BaseIMU::readX() {
  return (uint16_t)_readByte(BASE_IMU_OUT_X_H) << 8 |
         _readByte(BASE_IMU_OUT_X_L);
}

int16_t BaseIMU::readY() {
  return (uint16_t)_readByte(BASE_IMU_OUT_Y_H) << 8 |
         _readByte(BASE_IMU_OUT_Y_L);
}

int16_t BaseIMU::readZ() {
  return (uint16_t)_readByte(BASE_IMU_OUT_Z_H) << 8 |
         _readByte(BASE_IMU_OUT_Z_L);
}

void BaseIMU::readXYZ(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t data[6];
  _readBytes(0x80 | BASE_IMU_OUT_X_L, data, 6);
  x = (uint16_t)data[1] << 8 | (uint16_t)data[0];
  y = (uint16_t)data[3] << 8 | (uint16_t)data[2];
  z = (uint16_t)data[5] << 8 | (uint16_t)data[4];
}

uint8_t BaseIMU::_readByte(uint8_t regAddress) {
  uint8_t value;
  i2c_write_blocking(i2c0, _slaveAddress, &regAddress, 1,
                     true);
  i2c_read_blocking(i2c0, _slaveAddress, &value, 1, false);
  return value;
}

void BaseIMU::_writeByte(uint8_t regAddress,
                         uint8_t value) {
  uint8_t data[2] = {regAddress, value};
  i2c_write_blocking(i2c0, _slaveAddress, data,
                     sizeof(data), false);
}

void BaseIMU::_readBytes(uint8_t regAddress, uint8_t *data,
                         uint8_t length) {
  // Send the register address
  int result = i2c_write_blocking(i2c0, _slaveAddress,
                                  &regAddress, 1, true);
  if (result == PICO_ERROR_GENERIC) {
    // uart_puts(uart0, "write failed\n");
    return;
    // } else {
    //   uart_puts(uart0, "write success\n");
  }

  // Read the specified number of bytes into the provided
  // buffer
  result = i2c_read_blocking(i2c0, _slaveAddress, data,
                             length, false);
  if (result == PICO_ERROR_GENERIC) {
    // uart_puts(uart0, "read failed\n");
    return;
    // } else {
    //   uart_puts(uart0, "read success\n");
  }
}
