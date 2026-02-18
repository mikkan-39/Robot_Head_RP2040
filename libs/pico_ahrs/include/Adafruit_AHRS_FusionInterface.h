/*!
 * @file Adafruit_AHRS_FusionInterface.h
 *
 * The MIT License (MIT)
 * Copyright (c) 2020 Ha Thach (tinyusb.org) for Adafruit Industries
 */

#ifndef ADAFRUIT_AHRS_FUSIONINTERFACE_H_
#define ADAFRUIT_AHRS_FUSIONINTERFACE_H_

class Adafruit_AHRS_FusionInterface {
public:
  virtual void  begin(float sampleFrequency)                                                                    = 0;
  virtual void  update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) = 0;
  virtual float getRoll()                                                                                        = 0;
  virtual float getPitch()                                                                                       = 0;
  virtual float getYaw()                                                                                         = 0;
  virtual void  getQuaternion(float *w, float *x, float *y, float *z)                                           = 0;
  virtual void  setQuaternion(float w, float x, float y, float z)                                               = 0;
  virtual void  getGravityVector(float *x, float *y, float *z)                                                  = 0;
};

#endif
