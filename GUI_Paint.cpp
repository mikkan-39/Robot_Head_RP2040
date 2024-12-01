#include "GUI_Paint.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lcd.pio.h"

volatile PAINT Paint;

/******************************************************************************
  function: Create Image
  parameter:
    image   :   Pointer to the image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    Color   :   Whether the picture is inverted
******************************************************************************/
void Paint_NewImage(UWORD Width, UWORD Height, UWORD Color)
{
  Paint.WidthMemory = Width;
  Paint.HeightMemory = Height;
  Paint.Color = Color;
  Paint.WidthByte = Width;
  Paint.HeightByte = Height;
  Paint.Width = Width;
  Paint.Height = Height;
}

/******************************************************************************
function:	Draw a line of arbitrary slope
parameter:
    Xstart 锛歋tarting Xpoint point coordinates
    Ystart 锛歋tarting Xpoint point coordinates
    Xend   锛欵nd point Xpoint coordinate
    Yend   锛欵nd point Ypoint coordinate
    Color  锛歍he color of the line segment
******************************************************************************/
void Paint_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
{
    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
    
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    //Cumulative error
    int Esp = dx + dy;

    for (;;) {
        LCD_DrawPixel(Xpoint, Ypoint, Color);
        if (2 * Esp >= dy) {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx) {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

/******************************************************************************
function:	Draw a rectangle
parameter:
    Xstart 锛歊ectangular  Starting Xpoint point coordinates
    Ystart 锛歊ectangular  Starting Xpoint point coordinates
    Xend   锛歊ectangular  End point Xpoint coordinate
    Yend   锛歊ectangular  End point Ypoint coordinate
    Color  锛歍he color of the Rectangular segment
    Filled : Whether it is filled--- 1 solid 0锛歟mpty
******************************************************************************/
void Paint_DrawRectangle( UWORD X_Center, UWORD Y_Center, UWORD Radius, 
                          UWORD Color, DRAW_FILL Filled )
{
    int Xstart = std::max(X_Center-Radius-5, 0);
    int Ystart = std::max(Y_Center-Radius-5, 0);
    int Xend = std::min(X_Center+Radius+5, (int)Paint.Width);
    int Yend = std::min(Y_Center+Radius+5, (int)Paint.Height);

    if (Filled) {
        LCD_SetCursor(Xstart, Ystart, Xend, Yend);
        for(int y = Ystart; y<=Yend; y++) {
            for(int x = Xstart; x<=Xend; x++){
              if(x-Xstart>5 && y-Ystart>5 && Xend-x>5 && Yend-y>5){
                LCD_WriteData_Word(Color);
              }else{
                LCD_WriteData_Word(BLACK);
              }
            }
        }
    } else {
        Paint_DrawLine(Xstart, Ystart, Xend, Ystart, Color);
        Paint_DrawLine(Xstart, Ystart, Xstart, Yend, Color);
        Paint_DrawLine(Xend, Yend, Xend, Ystart, Color);
        Paint_DrawLine(Xend, Yend, Xstart, Yend, Color);
    }
}

/******************************************************************************
function:	Use the 8-point method to draw a circle of the
            specified size at the specified position->
parameter:
    X_Center  锛欳enter X coordinate
    Y_Center  锛欳enter Y coordinate
    Radius    锛歝ircle Radius
    Color     锛歍he color of the 锛歝ircle segment
    Filled    : Whether it is filled: 1 filling 0锛欴o not
******************************************************************************/
void Paint_DrawCircle(  UWORD X_Center, UWORD Y_Center, UWORD Radius, 
                        UWORD Color, DRAW_FILL Draw_Fill, UBYTE sideStep )
{
  int xoffset;
  int yoffset;
  int offs;

  int x_boundary_l = std::max(X_Center-Radius-sideStep, 0);
  int y_boundary_l = std::max(Y_Center-Radius-sideStep, 0);
  int x_boundary_h = std::min(X_Center+Radius+sideStep, (int)Paint.Width);
  int y_boundary_h = std::min(Y_Center+Radius+sideStep, (int)Paint.Height);

  LCD_SetCursor(x_boundary_l, y_boundary_l, x_boundary_h, y_boundary_h);

  if(Draw_Fill == DRAW_FILL_FULL){
    for(int y=y_boundary_l; y<=y_boundary_h; y++){
      for(int x=x_boundary_l; x<=x_boundary_h; x++){
        xoffset = X_Center-x;
        yoffset = Y_Center-y;
        if(xoffset*xoffset+yoffset*yoffset <= Radius*Radius){
          LCD_WriteData_Word(Color);
        } else {
          LCD_WriteData_Word(BLACK);
        }
      }
    } 
  } else {
    for(int y=y_boundary_l; y<=y_boundary_h; y++){
      for(int x=x_boundary_l; x<=x_boundary_h; x++){
        xoffset = X_Center-x;
        yoffset = Y_Center-y;
        offs = xoffset*xoffset + yoffset*yoffset - Radius*Radius;
        if(offs <= 0){
          if(offs > -Radius*10){
            LCD_WriteData_Word(Color);
          } else {
            LCD_WriteData_Word(BLACK);
          }
        } 
        else {
          LCD_WriteData_Word(BLACK);
        }
      }
    }
  }
}

