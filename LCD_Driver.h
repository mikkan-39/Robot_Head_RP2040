#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

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

void LCD_WriteData_Byte(UBYTE da); 
void LCD_WriteData_Word(UWORD da);
void LCD_WriteReg(UBYTE da);

void LCD_SetCursor(UWORD x1, UWORD y1, UWORD x2,UWORD y2);
void LCD_DrawPixel(UWORD x, UWORD y, UWORD Color);

void LCD_Init(void);
void LCD_Clear(UWORD Color);
void LCD_ClearWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD UWORD);

#endif