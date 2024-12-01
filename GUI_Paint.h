#ifndef __GUI_PAINT_H
#define __GUI_PAINT_H

#include "LCD_Driver.h"
/**
 * Image attributes
**/
typedef struct {
    UBYTE *Image;
    UWORD Width;
    UWORD Height;
    UWORD WidthMemory;
    UWORD HeightMemory;
    UWORD Color;
    UWORD WidthByte;
    UWORD HeightByte;
} PAINT;
extern volatile PAINT Paint;

/**
 * image color
**/

#define WHITE         0xFFFF
#define BLACK         0x0000    
#define BLUE          0x001F  
#define BRED          0XF81F
#define GRED          0XFFE0
#define GBLUE         0X07FF
#define RED           0xF800
#define MAGENTA       0xF81F
#define GREEN         0x07E0
#define CYAN          0x7FFF
#define YELLOW        0xFFE0
#define BROWN         0XBC40 
#define BRRED         0XFC07 
#define GRAY          0X8430 
#define DARKBLUE      0X01CF  
#define LIGHTBLUE     0X7D7C   
#define GRAYBLUE      0X5458 
#define LIGHTGREEN    0X841F 
#define LGRAY         0XC618 
#define LGRAYBLUE     0XA651
#define LBBLUE        0X2B12 


#define IMAGE_BACKGROUND    WHITE
#define FONT_FOREGROUND     BLACK
#define FONT_BACKGROUND     WHITE

/**
 * Whether the graphic is filled
**/
typedef enum {
    DRAW_FILL_EMPTY = 0,
    DRAW_FILL_FULL,
} DRAW_FILL;

//init and Clear
void Paint_NewImage(UWORD Width, UWORD Height, UWORD Color);
void Paint_SelectImage(UBYTE *image);

//Drawing
void Paint_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color);
void Paint_DrawRectangle(UWORD X_Center, UWORD Y_Center, UWORD Radius, UWORD Color, DRAW_FILL Filled);
void Paint_DrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius, UWORD Color, DRAW_FILL Draw_Fill, UBYTE sideStep);

//pic
void Paint_DrawImage(const unsigned char *image,UWORD Startx, UWORD Starty,UWORD Endx, UWORD Endy); 


#endif

