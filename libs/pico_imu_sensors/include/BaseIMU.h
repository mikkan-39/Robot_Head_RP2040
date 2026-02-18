#ifndef __BASE_IMU_H__
#define __BASE_IMU_H__

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdint.h>

constexpr uint8_t BASE_IMU_WHO_AM_I  = 0x0F;
constexpr uint8_t BASE_IMU_CTRL_REG1 = 0x20;
constexpr uint8_t BASE_IMU_CTRL_REG2 = 0x21;
constexpr uint8_t BASE_IMU_CTRL_REG3 = 0x22;
constexpr uint8_t BASE_IMU_CTRL_REG4 = 0x23;
constexpr uint8_t BASE_IMU_CTRL_REG5 = 0x24;

constexpr uint8_t BASE_IMU_OUT_X_L = 0x28;
constexpr uint8_t BASE_IMU_OUT_X_H = 0x29;
constexpr uint8_t BASE_IMU_OUT_Y_L = 0x2A;
constexpr uint8_t BASE_IMU_OUT_Y_H = 0x2B;
constexpr uint8_t BASE_IMU_OUT_Z_L = 0x2C;
constexpr uint8_t BASE_IMU_OUT_Z_H = 0x2D;

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif
constexpr float TWO_PI    = 6.283185307179586476925286766559f;
constexpr float RAD_TO_DEG = 180.0f / M_PI;
constexpr float DEG_TO_RAD = M_PI / 180.0f;

class BaseIMU {
public:
  // wire defaults to i2c0; pass i2c1 for the other bus
  BaseIMU(uint8_t slaveAddress, i2c_inst_t *wire = i2c0)
      : _slaveAddress(slaveAddress), _wire(wire) {}

  uint8_t readDeviceID();
  int16_t readX();
  int16_t readY();
  int16_t readZ();
  void    readXYZ(int16_t &x, int16_t &y, int16_t &z);

protected:
  uint8_t    _readByte(uint8_t regAddress);
  void       _writeByte(uint8_t regAddress, uint8_t data);
  void       _readBytes(uint8_t regAddress, uint8_t *data, uint8_t length);
  i2c_inst_t *_wire;

private:
  uint8_t _slaveAddress;
};

#endif // __BASE_IMU_H__
