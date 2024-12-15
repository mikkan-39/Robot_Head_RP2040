#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

#define LCD_WIDTH 240
#define LCD_HEIGHT 240

#define DEV_CS_PIN_RIGHT 5
#define DEV_DC_PIN_RIGHT 4
#define DEV_MOSI_PIN_RIGHT 3
#define DEV_SCK_PIN_RIGHT 2

#define DEV_CS_PIN_LEFT 9
#define DEV_DC_PIN_LEFT 8
#define DEV_MOSI_PIN_LEFT 7
#define DEV_SCK_PIN_LEFT 6

#define DEV_RST_PIN 10

#define pio_state_machine 0
#define pio_instance_right pio0
#define pio_instance_left pio1

// void SelectScreenR();
// void SelectScreenL();
// void SelectBothScreens();
// void DeselectBothScreens();

void LCD_WriteData_Byte(PIO pio_instance, uint8_t da);
void LCD_WriteData_Byte_Both(uint8_t da);
void LCD_WriteData_Word(PIO pio_instance, uint16_t da);
void LCD_Both_WriteReg(uint8_t da);

void LCD_Both_Init(void);
void LCD_Both_SetCursor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif