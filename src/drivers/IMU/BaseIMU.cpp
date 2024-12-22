#include "BaseIMU.h"

uint8_t BaseIMU::readDeviceID() { return _readByte(BASE_IMU_WHO_AM_I); }

int16_t BaseIMU::readX() {
  return (static_cast<uint16_t>(_readByte(BASE_IMU_OUT_X_H)) << 8) |
         _readByte(BASE_IMU_OUT_X_L);
}

int16_t BaseIMU::readY() {
  return (static_cast<uint16_t>(_readByte(BASE_IMU_OUT_Y_H)) << 8) |
         _readByte(BASE_IMU_OUT_Y_L);
}

int16_t BaseIMU::readZ() {
  return (static_cast<uint16_t>(_readByte(BASE_IMU_OUT_Z_H)) << 8) |
         _readByte(BASE_IMU_OUT_Z_L);
}

void BaseIMU::readXYZ(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t data[6];
  _readBytes(0x80 | BASE_IMU_OUT_X_L, data, 6);
  x = (static_cast<uint16_t>(data[1]) << 8) | data[0];
  y = (static_cast<uint16_t>(data[3]) << 8) | data[2];
  z = (static_cast<uint16_t>(data[5]) << 8) | data[4];
}

uint8_t BaseIMU::_readByte(uint8_t regAddress) {
  uint8_t value;
  i2c_write_blocking(i2c0, _slaveAddress, &regAddress, 1, true);
  i2c_read_blocking(i2c0, _slaveAddress, &value, 1, false);
  return value;
}

void BaseIMU::_writeByte(uint8_t regAddress, uint8_t value) {
  uint8_t data[2] = {regAddress, value};
  i2c_write_blocking(i2c0, _slaveAddress, data, sizeof(data), false);
}

void BaseIMU::_readBytes(uint8_t regAddress, uint8_t *data, uint8_t length) {
  // Send the register address
  int result = i2c_write_blocking(i2c0, _slaveAddress, &regAddress, 1, true);
  if (result != 1) {
    // Handle error if the write operation fails
    printf("I2C write failed\n");
    return;
  }

  // Read the specified number of bytes into the provided buffer
  result = i2c_read_blocking(i2c0, _slaveAddress, data, length, false);
  if (result != length) {
    // Handle error if the read operation fails
    printf("I2C read failed\n");
  }
}
