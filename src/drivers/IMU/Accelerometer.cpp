#include "drivers/IMU/Accelerometer.h"

Accelerometer::Accelerometer(uint8_t slaveAddress)
    : BaseIMU(slaveAddress) {}

void Accelerometer::begin() {
  _scalingFactor = 1;
  uint8_t command =
      LIS331DLH_CTRL_REG1_X_EN | LIS331DLH_CTRL_REG1_Y_EN |
      LIS331DLH_CTRL_REG1_Z_EN | LIS331DLH_CTRL_REG1_PM0;

  uint8_t data[2] = {BASE_IMU_CTRL_REG1, command};
  i2c_write_blocking(i2c0, LIS331DLH_SLAVE_ADDRESS, data,
                     sizeof(data), false);

  setRange(AccelerometerRange::RANGE_2G);
}

// Set range scale output data from datasheet
void Accelerometer::setRange(AccelerometerRange range) {
  uint8_t data = _readByte(BASE_IMU_CTRL_REG4);
  data &=
      ~(LIS331DLH_CTRL_REG4_FS0 | LIS331DLH_CTRL_REG4_FS1);
  switch (range) {
  case AccelerometerRange::RANGE_2G: {
    _scalingFactor = SENS_2G;
    break;
  }
  case AccelerometerRange::RANGE_4G: {
    data |= LIS331DLH_CTRL_REG4_FS0;
    _scalingFactor = SENS_4G;
    break;
  }
  case AccelerometerRange::RANGE_8G: {
    data |=
        LIS331DLH_CTRL_REG4_FS0 | LIS331DLH_CTRL_REG4_FS1;
    _scalingFactor = SENS_8G;
    break;
  }
  default: {
    _scalingFactor = SENS_2G;
  } break;
  }
  _writeByte(BASE_IMU_CTRL_REG4, data);
}

void Accelerometer::sleep(bool state) {
  uint8_t data = _readByte(BASE_IMU_CTRL_REG1);
  if (state)
    data &= ~LIS331DLH_CTRL_REG1_PM0;
  else
    data |= LIS331DLH_CTRL_REG1_PM0;

  _writeByte(BASE_IMU_CTRL_REG1, data);
}

float Accelerometer::readAccelerationGX() {
  return readX() / _scalingFactor;
}

float Accelerometer::readAccelerationGY() {
  return readY() / _scalingFactor;
}

float Accelerometer::readAccelerationGZ() {
  return readZ() / _scalingFactor;
}

float Accelerometer::readAccelerationAX() {
  return readAccelerationGX() * GRAVITY_EARTH;
}

float Accelerometer::readAccelerationAY() {
  return readAccelerationGY() * GRAVITY_EARTH;
}

float Accelerometer::readAccelerationAZ() {
  return readAccelerationGZ() * GRAVITY_EARTH;
}

void Accelerometer::readAccelerationGXYZ(float &ax,
                                         float &ay,
                                         float &az) {
  int16_t x, y, z;
  readXYZ(x, y, z);
  ax = (float)x / _scalingFactor;
  ay = (float)y / _scalingFactor;
  az = (float)z / _scalingFactor;
}

void Accelerometer::readAccelerationAXYZ(float &ax,
                                         float &ay,
                                         float &az) {
  readAccelerationGXYZ(ax, ay, az);
  ax *= GRAVITY_EARTH;
  ay *= GRAVITY_EARTH;
  az *= GRAVITY_EARTH;
}
