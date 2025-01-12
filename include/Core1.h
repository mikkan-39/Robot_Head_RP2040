#ifndef _CORE1_H_
#define _CORE1_H_

#include "drivers/AdafruitAHRS/Adafruit_AHRS.h"
#include "drivers/GpioUtils.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "multicore/CommandParsers.h"
#include "multicore/MulticoreUtils.h"
#include "multicore/UART_Utils.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <cstdio>

void core1_thread();

#endif