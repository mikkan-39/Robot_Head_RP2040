#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#define LCD_WIDTH   240
#define LCD_HEIGHT  240

#define DEV_CS_PIN  6
#define DEV_CS_PIN_2  7

#define DEV_DC_PIN  8
#define DEV_RST_PIN 10
#define DEV_SCK_PIN  2
#define DEV_MOSI_PIN 3

#define pio_state_machine 0

void SelectScreenR();
void SelectScreenL();
void SelectBothScreens();

void LCD_WriteData_Byte(uint8_t da); 
void LCD_WriteData_Word(uint16_t da);
void LCD_WriteReg(uint8_t da);

void LCD_Init(void);
void LCD_Clear(uint16_t Color);
void LCD_SetCursor(uint16_t x1, uint16_t y1, uint16_t x2,uint16_t y2);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t Color);


#endif