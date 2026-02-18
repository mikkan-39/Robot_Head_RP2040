// Madgwick's implementation of Mahony's AHRS algorithm.
// http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
// GNU General Public Licence

#include "Adafruit_AHRS_Mahony.h"
#include <math.h>

#define DEFAULT_SAMPLE_FREQ 512.0f
#define twoKpDef (2.0f * 0.5f)
#define twoKiDef (2.0f * 0.0f)

Adafruit_Mahony::Adafruit_Mahony() : Adafruit_Mahony(twoKpDef, twoKiDef) {}

Adafruit_Mahony::Adafruit_Mahony(float prop_gain, float int_gain) {
  twoKp = prop_gain;
  twoKi = int_gain;
  q0    = 1.0f; q1 = q2 = q3 = 0.0f;
  integralFBx = integralFBy = integralFBz = 0.0f;
  anglesComputed = false;
  invSampleFreq  = 1.0f / DEFAULT_SAMPLE_FREQ;
}

void Adafruit_Mahony::update(float gx, float gy, float gz,
                             float ax, float ay, float az,
                             float mx, float my, float mz,
                             float dt) {
  float recipNorm;
  float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
  float hx, hy, bx, bz;
  float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
  float halfex, halfey, halfez;
  float qa, qb, qc;

  if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
    updateIMU(gx, gy, gz, ax, ay, az);
    return;
  }

  gx *= 0.0174533f; gy *= 0.0174533f; gz *= 0.0174533f;

  if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
    recipNorm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm; ay *= recipNorm; az *= recipNorm;

    recipNorm = invSqrt(mx * mx + my * my + mz * mz);
    mx *= recipNorm; my *= recipNorm; mz *= recipNorm;

    q0q0 = q0 * q0; q0q1 = q0 * q1; q0q2 = q0 * q2; q0q3 = q0 * q3;
    q1q1 = q1 * q1; q1q2 = q1 * q2; q1q3 = q1 * q3;
    q2q2 = q2 * q2; q2q3 = q2 * q3; q3q3 = q3 * q3;

    hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
    hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
    bx = sqrtf(hx * hx + hy * hy);
    bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

    halfvx = q1q3 - q0q2;
    halfvy = q0q1 + q2q3;
    halfvz = q0q0 - 0.5f + q3q3;
    halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
    halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
    halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);

    halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
    halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
    halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

    if (twoKi > 0.0f) {
      integralFBx += twoKi * halfex * dt;
      integralFBy += twoKi * halfey * dt;
      integralFBz += twoKi * halfez * dt;
      gx += integralFBx; gy += integralFBy; gz += integralFBz;
    } else {
      integralFBx = integralFBy = integralFBz = 0.0f;
    }

    gx += twoKp * halfex;
    gy += twoKp * halfey;
    gz += twoKp * halfez;
  }

  gx *= (0.5f * dt); gy *= (0.5f * dt); gz *= (0.5f * dt);
  qa = q0; qb = q1; qc = q2;
  q0 += (-qb * gx - qc * gy - q3 * gz);
  q1 += ( qa * gx + qc * gz - q3 * gy);
  q2 += ( qa * gy - qb * gz + q3 * gx);
  q3 += ( qa * gz + qb * gy - qc * gx);

  recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
  q0 *= recipNorm; q1 *= recipNorm; q2 *= recipNorm; q3 *= recipNorm;
  anglesComputed = 0;
}

void Adafruit_Mahony::updateIMU(float gx, float gy, float gz,
                                float ax, float ay, float az,
                                float dt) {
  float recipNorm;
  float halfvx, halfvy, halfvz, halfex, halfey, halfez;
  float qa, qb, qc;

  gx *= 0.0174533f; gy *= 0.0174533f; gz *= 0.0174533f;

  if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
    recipNorm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm; ay *= recipNorm; az *= recipNorm;

    halfvx = q1 * q3 - q0 * q2;
    halfvy = q0 * q1 + q2 * q3;
    halfvz = q0 * q0 - 0.5f + q3 * q3;

    halfex = ay * halfvz - az * halfvy;
    halfey = az * halfvx - ax * halfvz;
    halfez = ax * halfvy - ay * halfvx;

    if (twoKi > 0.0f) {
      integralFBx += twoKi * halfex * dt;
      integralFBy += twoKi * halfey * dt;
      integralFBz += twoKi * halfez * dt;
      gx += integralFBx; gy += integralFBy; gz += integralFBz;
    } else {
      integralFBx = integralFBy = integralFBz = 0.0f;
    }

    gx += twoKp * halfex;
    gy += twoKp * halfey;
    gz += twoKp * halfez;
  }

  gx *= (0.5f * dt); gy *= (0.5f * dt); gz *= (0.5f * dt);
  qa = q0; qb = q1; qc = q2;
  q0 += (-qb * gx - qc * gy - q3 * gz);
  q1 += ( qa * gx + qc * gz - q3 * gy);
  q2 += ( qa * gy - qb * gz + q3 * gx);
  q3 += ( qa * gz + qb * gy - qc * gx);

  recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
  q0 *= recipNorm; q1 *= recipNorm; q2 *= recipNorm; q3 *= recipNorm;
  anglesComputed = 0;
}

void Adafruit_Mahony::update(float gx, float gy, float gz,
                             float ax, float ay, float az,
                             float mx, float my, float mz) {
  update(gx, gy, gz, ax, ay, az, mx, my, mz, invSampleFreq);
}

void Adafruit_Mahony::updateIMU(float gx, float gy, float gz,
                                float ax, float ay, float az) {
  updateIMU(gx, gy, gz, ax, ay, az, invSampleFreq);
}

float Adafruit_Mahony::invSqrt(float x) {
  float halfx = 0.5f * x;
  union { float f; long i; } conv = {x};
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f *= 1.5f - (halfx * conv.f * conv.f);
  conv.f *= 1.5f - (halfx * conv.f * conv.f);
  return conv.f;
}

void Adafruit_Mahony::computeAngles() {
  roll           = atan2f(q0 * q1 + q2 * q3, 0.5f - q1 * q1 - q2 * q2);
  pitch          = asinf(-2.0f * (q1 * q3 - q0 * q2));
  yaw            = atan2f(q1 * q2 + q0 * q3, 0.5f - q2 * q2 - q3 * q3);
  grav[0]        = 2.0f * (q1 * q3 - q0 * q2);
  grav[1]        = 2.0f * (q0 * q1 + q2 * q3);
  grav[2]        = 2.0f * (q1 * q0 - 0.5f + q3 * q3);
  anglesComputed = 1;
}
