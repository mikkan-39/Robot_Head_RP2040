#ifndef __COMPASS_H__
#define __COMPASS_H__

#include "BaseIMU.h"
#include <cstring>

constexpr uint8_t LIS3MDL_SLAVE_ADDRESS     = 0x1C;
constexpr uint8_t LIS3MDL_SLAVE_ADDRESS_ALT = 0x1E;

constexpr uint8_t LIS3MDL_CTRL_REG2_FS0 = 0x20;
constexpr uint8_t LIS3MDL_CTRL_REG2_FS1 = 0x40;

constexpr uint8_t LIS3MDL_CTRL_REG3_MD0 = 0x01;
constexpr uint8_t LIS3MDL_CTRL_REG3_MD1 = 0x02;

constexpr float SENS_4GAUSS  = 6842.0f;
constexpr float SENS_8GAUSS  = 3421.0f;
constexpr float SENS_12GAUSS = 2281.0f;
constexpr float SENS_16GAUSS = 1711.0f;

enum class CompassRange { RANGE_4GAUSS = 1, RANGE_8GAUSS = 2, RANGE_12GAUSS = 3, RANGE_16GAUSS = 4 };

class Compass : public BaseIMU {
public:
  Compass(uint8_t slaveAddress = LIS3MDL_SLAVE_ADDRESS,
          i2c_inst_t *wire = i2c0)
      : BaseIMU(slaveAddress, wire) {}

  void begin();
  void sleep(bool state);
  void setRange(CompassRange range);

  float readMagneticGaussX();
  float readMagneticGaussY();
  float readMagneticGaussZ();
  void  readMagneticGaussXYZ(float &mx, float &my, float &mz);
  void  readCalibrateMagneticGaussXYZ(float &mx, float &my, float &mz);
  void  setCalibrateMatrix(const float calibrationMatrix[3][3], const float bias[3]);
  float readAzimut();

private:
  void  _calibrate(float &mx, float &my, float &mz);
  float _calibrationMatrix[3][3];
  float _calibrationBias[3];
  float _scalingFactor;
};

#endif // __COMPASS_H__
