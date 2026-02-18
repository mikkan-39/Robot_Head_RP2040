#include "Compass.h"

void Compass::begin() {
  _scalingFactor = 1;
  uint8_t data = 0;
  data &= ~(LIS3MDL_CTRL_REG3_MD0 | LIS3MDL_CTRL_REG3_MD1);
  _writeByte(BASE_IMU_CTRL_REG3, data);
  setRange(CompassRange::RANGE_4GAUSS);
}

void Compass::setRange(CompassRange range) {
  uint8_t data = _readByte(BASE_IMU_CTRL_REG2);
  data &= ~(LIS3MDL_CTRL_REG2_FS0 | LIS3MDL_CTRL_REG2_FS1);
  switch (range) {
  case CompassRange::RANGE_4GAUSS:
    _scalingFactor = SENS_4GAUSS;
    break;
  case CompassRange::RANGE_8GAUSS:
    data |= LIS3MDL_CTRL_REG2_FS0;
    _scalingFactor = SENS_8GAUSS;
    break;
  case CompassRange::RANGE_12GAUSS:
    data |= LIS3MDL_CTRL_REG2_FS1;
    _scalingFactor = SENS_12GAUSS;
    break;
  case CompassRange::RANGE_16GAUSS:
    data |= LIS3MDL_CTRL_REG2_FS0 | LIS3MDL_CTRL_REG2_FS1;
    _scalingFactor = SENS_16GAUSS;
    break;
  default:
    _scalingFactor = SENS_4GAUSS;
    break;
  }
  _writeByte(BASE_IMU_CTRL_REG2, data);
}

void Compass::sleep(bool state) {
  uint8_t data = _readByte(BASE_IMU_CTRL_REG3);
  if (state)
    data |= LIS3MDL_CTRL_REG3_MD0 | LIS3MDL_CTRL_REG3_MD1;
  else
    data &= ~(LIS3MDL_CTRL_REG3_MD0 | LIS3MDL_CTRL_REG3_MD1);
  _writeByte(BASE_IMU_CTRL_REG3, data);
}

float Compass::readMagneticGaussX() { return readX() / _scalingFactor; }
float Compass::readMagneticGaussY() { return readY() / _scalingFactor; }
float Compass::readMagneticGaussZ() { return readZ() / _scalingFactor; }

void Compass::readMagneticGaussXYZ(float &mx, float &my, float &mz) {
  int16_t x, y, z;
  readXYZ(x, y, z);
  mx = x / _scalingFactor;
  my = y / _scalingFactor;
  mz = z / _scalingFactor;
}

void Compass::readCalibrateMagneticGaussXYZ(float &mx, float &my, float &mz) {
  int16_t x, y, z;
  readXYZ(x, y, z);
  mx = (float)x;
  my = (float)y;
  mz = (float)z;
  _calibrate(mx, my, mz);
  mx /= _scalingFactor;
  my /= _scalingFactor;
  mz /= _scalingFactor;
}

void Compass::setCalibrateMatrix(const float calibrationMatrix[3][3],
                                  const float calibrationBias[3]) {
  memcpy(_calibrationBias,   calibrationBias,   3 * sizeof(float));
  memcpy(_calibrationMatrix, calibrationMatrix, 9 * sizeof(float));
}

void Compass::_calibrate(float &x, float &y, float &z) {
  float cal[3] = {0, 0, 0};
  float raw[3] = {x - _calibrationBias[0],
                  y - _calibrationBias[1],
                  z - _calibrationBias[2]};
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      cal[i] += _calibrationMatrix[i][j] * raw[j];
  x = cal[0];
  y = cal[1];
  z = cal[2];
}

float Compass::readAzimut() {
  int16_t x, y, z;
  readXYZ(x, y, z);
  float mx = (float)x, my = (float)y, mz = (float)z;
  _calibrate(mx, my, mz);
  float heading = atan2(mx, my);
  if (heading < 0) heading += TWO_PI;
  return heading * RAD_TO_DEG;
}
