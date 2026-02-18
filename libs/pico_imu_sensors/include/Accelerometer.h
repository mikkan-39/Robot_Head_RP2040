#ifndef __ACCELEROMETER_H__
#define __ACCELEROMETER_H__

#include "BaseIMU.h"

constexpr uint8_t LIS331DLH_SLAVE_ADDRESS     = 0x18;
constexpr uint8_t LIS331DLH_SLAVE_ADDRESS_ALT = 0x19;

constexpr uint8_t LIS331DLH_CTRL_REG4_FS0 = 0x10;
constexpr uint8_t LIS331DLH_CTRL_REG4_FS1 = 0x20;

constexpr uint8_t LIS331DLH_CTRL_REG1_X_EN = 0x01;
constexpr uint8_t LIS331DLH_CTRL_REG1_Y_EN = 0x02;
constexpr uint8_t LIS331DLH_CTRL_REG1_Z_EN = 0x04;
constexpr uint8_t LIS331DLH_CTRL_REG1_PM0  = 0x20;
constexpr uint8_t LIS331DLH_CTRL_REG1_PM1  = 0x40;
constexpr uint8_t LIS331DLH_CTRL_REG1_PM2  = 0x80;

constexpr float GRAVITY_EARTH = 9.8f;

constexpr float SENS_2G = 16384.0f;
constexpr float SENS_4G = 32768.0f;
constexpr float SENS_8G = 65536.0f;

enum class AccelerometerRange { RANGE_2G = 1, RANGE_4G = 2, RANGE_8G = 3 };

class Accelerometer : public BaseIMU {
public:
  Accelerometer(uint8_t slaveAddress = LIS331DLH_SLAVE_ADDRESS,
                i2c_inst_t *wire = i2c0)
      : BaseIMU(slaveAddress, wire) {}

  void begin();
  void sleep(bool state);
  void setRange(AccelerometerRange range);

  float readAccelerationGX();
  float readAccelerationGY();
  float readAccelerationGZ();
  float readAccelerationAX();
  float readAccelerationAY();
  float readAccelerationAZ();
  void  readAccelerationGXYZ(float &ax, float &ay, float &az);
  void  readAccelerationAXYZ(float &ax, float &ay, float &az);

private:
  float _scalingFactor;
};

#endif // __ACCELEROMETER_H__
