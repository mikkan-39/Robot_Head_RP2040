/*!
 * @file Adafruit_AHRS_NXPFusion.h
 *
 * Kalman/NXP 9DOF sensor fusion algorithm.
 * Based on https://github.com/memsindustrygroup/Open-Source-Sensor-Fusion
 * Modified by PJRC / Paul Stoffregen
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * BSD 3-Clause License
 */

#ifndef __Adafruit_Nxp_Fusion_h_
#define __Adafruit_Nxp_Fusion_h_

#include "Adafruit_AHRS_FusionInterface.h"
#include <stdint.h>

class Adafruit_NXPSensorFusion : public Adafruit_AHRS_FusionInterface {
public:
  void begin(float sampleFrequency = 100.0f);
  void update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

  float getRoll()  { return PhiPl; }
  float getPitch() { return ThePl; }
  float getYaw()   { return PsiPl; }

  void getQuaternion(float *w, float *x, float *y, float *z) {
    *w = qPl.q0; *x = qPl.q1; *y = qPl.q2; *z = qPl.q3;
  }
  void setQuaternion(float w, float x, float y, float z) {
    qPl.q0 = w; qPl.q1 = x; qPl.q2 = y; qPl.q3 = z;
  }
  void getGravityVector(float *x, float *y, float *z) {
    *x = gSeGyMi[0]; *y = gSeGyMi[1]; *z = gSeGyMi[2];
  }
  void getLinearAcceleration(float *x, float *y, float *z) const {
    *x = aSePl[0]; *y = aSePl[1]; *z = aSePl[2];
  }
  void getGeomagneticVector(float *x, float *y, float *z) const {
    *x = mGl[0]; *y = mGl[1]; *z = mGl[2];
  }

  typedef struct { float q0, q1, q2, q3; } Quaternion_t;

private:
  float      PhiPl, ThePl, PsiPl, RhoPl, ChiPl;
  float      RPl[3][3];
  Quaternion_t qPl;
  float      RVecPl[3];
  float      Omega[3];
  int32_t    systick;
  float      bPl[3], ThErrPl[3], bErrPl[3];
  float      dErrGlPl[3], dErrSePl[3], aErrSePl[3];
  float      aSeMi[3], DeltaPl, aSePl[3], aGlPl[3];
  float      gErrSeMi[3], mErrSeMi[3], gSeGyMi[3], mSeGyMi[3], mGl[3];
  float      QvAA, QvMM;
  float      PPlus12x12[12][12];
  float      K12x6[12][6];
  float      Qw12x12[12][12];
  float      C6x12[6][12];
  float      RMi[3][3];
  Quaternion_t Deltaq, qMi;
  float      casq, cdsq, Fastdeltat, deltat, deltatsq, QwbplusQvG;
  int8_t     FirstOrientationLock, resetflag;
};

#endif
