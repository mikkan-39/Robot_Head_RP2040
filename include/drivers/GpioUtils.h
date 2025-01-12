#ifndef _GPIOUTILS_H_
#define _GPIOUTILS_H_

#include "IMU/IMU.h"
#include "LCD_Driver.h"
#include "TOF_Driver.h"
#include "lcd.pio.h"

#define I2C_PORT i2c0
#define UART_ID uart0

extern Gyroscope gyroscope;
extern Accelerometer accelerometer;
extern Compass compass;

void InitAllGpio();

#endif