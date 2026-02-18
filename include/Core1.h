#ifndef _CORE1_H_
#define _CORE1_H_

#include "drivers/GpioUtils.h"
#include "multicore/MulticoreUtils.h"
#include "multicore/UART_Utils.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

void core1_thread();

#endif
