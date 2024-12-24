#ifndef _CORE1_H_
#define _CORE1_H_

#include "MulticoreUtils.h"
#include "UART_Utils.h"
#include "drivers/GpioUtils.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <cstdio>

void core1_thread();

#endif