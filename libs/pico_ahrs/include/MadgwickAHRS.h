#ifndef __MADGWICK_AHRS_H__
#define __MADGWICK_AHRS_H__

#include <math.h>

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif
constexpr float AHRS_RAD_TO_DEG = 180.0f / M_PI;
constexpr float AHRS_DEG_TO_RAD = M_PI / 180.0f;

// Gyroscope measurement error in rads/s (start at 40 deg/s)
constexpr float GYRO_MEAS_ERROR = M_PI * (40.0f / 180.0f);
// Gyroscope measurement drift in rad/s/s
constexpr float GYRO_MEAS_DRIFT = M_PI * (0.0f / 180.0f);
constexpr float BETA_DEFAULT    = 0.866025f * GYRO_MEAS_ERROR;
constexpr float ZETA_DEFAULT    = 0.866025f * GYRO_MEAS_DRIFT;

class Madgwick {
public:
  Madgwick();
  void begin();
  void reset();
  void setSettings(float beta = BETA_DEFAULT, float zeta = ZETA_DEFAULT);
  void setFrequency(float frequency);
  void readQuaternion(float &q0, float &q1, float &q2, float &q3);

  void update(float gx, float gy, float gz,
              float ax, float ay, float az,
              float mx, float my, float mz);
  void update(float gx, float gy, float gz,
              float ax, float ay, float az);

  float getPitchRad();
  float getRollRad();
  float getYawRad();
  float getPitchDeg();
  float getRollDeg();
  float getYawDeg();

private:
  float _beta, _zeta, _frequency;
  float _q0, _q1, _q2, _q3;
};

#endif // __MADGWICK_AHRS_H__
