#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <stdint.h>
#include <stdio.h>
#include "Debug.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * GPIO config
**/

#define DEV_CS_PIN  6
#define DEV_CS_PIN_2  7
#define DEV_DC_PIN  8
#define DEV_RST_PIN 10

/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) gpio_put(_pin, _value)
#define DEV_Digital_Read(_pin) gpio_get(_pin)


/**
 * SPI
**/
#define DEV_SPI_WRITE(_dat)   spi_write_blocking(spi0, _dat, 1)
#define DEV_SPI_WRITE_WORD(_dat)   spi_write16_blocking(spi0, _dat, 1)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms)    sleep_ms(__xms)

/*-----------------------------------------------------------------------------*/
 void Config_Init();

#endif
