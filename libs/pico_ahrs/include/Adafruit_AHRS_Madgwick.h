// Madgwick's IMU and AHRS algorithms.
// http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
// GNU General Public Licence

#ifndef __Adafruit_Madgwick_h__
#define __Adafruit_Madgwick_h__

#include "Adafruit_AHRS_FusionInterface.h"
#include <math.h>

class Adafruit_Madgwick : public Adafruit_AHRS_FusionInterface {
public:
  Adafruit_Madgwick();
  Adafruit_Madgwick(float gain);

  void  begin(float sampleFrequency) { invSampleFreq = 1.0f / sampleFrequency; }
  void  update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
  void  updateIMU(float gx, float gy, float gz, float ax, float ay, float az);
  void  update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt);
  void  updateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt);

  float getBeta() { return beta; }
  void  setBeta(float b) { beta = b; }

  float getRoll()  { if (!anglesComputed) computeAngles(); return roll  * 57.29578f; }
  float getPitch() { if (!anglesComputed) computeAngles(); return pitch * 57.29578f; }
  float getYaw()   { if (!anglesComputed) computeAngles(); return yaw   * 57.29578f + 180.0f; }

  float getRollRadians()  { if (!anglesComputed) computeAngles(); return roll; }
  float getPitchRadians() { if (!anglesComputed) computeAngles(); return pitch; }
  float getYawRadians()   { if (!anglesComputed) computeAngles(); return yaw; }

  void getQuaternion(float *w, float *x, float *y, float *z) {
    *w = q0; *x = q1; *y = q2; *z = q3;
  }
  void setQuaternion(float w, float x, float y, float z) {
    q0 = w; q1 = x; q2 = y; q3 = z;
  }
  void getGravityVector(float *x, float *y, float *z) {
    if (!anglesComputed) computeAngles();
    *x = grav[0]; *y = grav[1]; *z = grav[2];
  }

private:
  static float invSqrt(float x);
  void         computeAngles();

  float beta;
  float q0, q1, q2, q3;
  float invSampleFreq;
  float roll, pitch, yaw;
  float grav[3];
  bool  anglesComputed;
};

#endif
