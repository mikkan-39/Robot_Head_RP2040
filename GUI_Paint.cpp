#include "GUI_Paint.h"
#include "DEV_Config.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

volatile PAINT Paint;

/******************************************************************************
  function: Create Image
  parameter:
    image   :   Pointer to the image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    Color   :   Whether the picture is inverted
******************************************************************************/
void Paint_NewImage(UWORD Width, UWORD Height, UWORD Rotate, UWORD Color)
{
  Paint.WidthMemory = Width;
  Paint.HeightMemory = Height;
  Paint.Color = Color;
  Paint.WidthByte = Width;
  Paint.HeightByte = Height;
  
  Paint.Rotate = Rotate;
  Paint.Mirror = MIRROR_NONE;

  if (Rotate == ROTATE_0 || Rotate == ROTATE_180) {
    Paint.Width = Width;
    Paint.Height = Height;
  } else {
    Paint.Width = Height;
    Paint.Height = Width;
  }
}

/******************************************************************************
  function: Select Image Rotate
  parameter:
    Rotate   :   0,90,180,270
******************************************************************************/
void Paint_SetRotate(UWORD Rotate)
{
  if (Rotate == ROTATE_0 || Rotate == ROTATE_90 || Rotate == ROTATE_180 || Rotate == ROTATE_270) {
    //Debug("Set image Rotate %d\r\n", Rotate);
    Paint.Rotate = Rotate;
  } else {
    //Debug("rotate = 0, 90, 180, 270\r\n");
    //  exit(0);
  }
}

/******************************************************************************
  function: Select Image mirror
  parameter:
    mirror   :       Not mirror,Horizontal mirror,Vertical mirror,Origin mirror
******************************************************************************/
void Paint_SetMirroring(UBYTE mirror)
{
  if (mirror == MIRROR_NONE || mirror == MIRROR_HORIZONTAL ||
      mirror == MIRROR_VERTICAL || mirror == MIRROR_ORIGIN) {
    //Debug("mirror image x:%s, y:%s\r\n", (mirror & 0x01) ? "mirror" : "none", ((mirror >> 1) & 0x01) ? "mirror" : "none");
    Paint.Mirror = mirror;
  } else {
    //Debug("mirror should be MIRROR_NONE, MIRROR_HORIZONTAL, \
        MIRROR_VERTICAL or MIRROR_ORIGIN\r\n");
    //exit(0);
  }
}

/******************************************************************************
  function: Draw Pixels
  parameter:
    Xpoint  :   At point X
    Ypoint  :   At point Y
    Color   :   Painted colors
******************************************************************************/
void Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
  if (Xpoint > Paint.Width || Ypoint > Paint.Height) {
    //Debug("Exceeding display boundaries\r\n");
    return;
  }
  UWORD X, Y;

  switch (Paint.Rotate) {
    case 0:
      X = Xpoint;
      Y = Ypoint;
      break;
    case 90:
      X = Paint.WidthMemory - Ypoint - 1;
      Y = Xpoint;
      break;
    case 180:
      X = Paint.WidthMemory - Xpoint - 1;
      Y = Paint.HeightMemory - Ypoint - 1;
      break;
    case 270:
      X = Ypoint;
      Y = Paint.HeightMemory - Xpoint - 1;
      break;

    default:
      return;
  }

  switch (Paint.Mirror) {
    case MIRROR_NONE:
      break;
    case MIRROR_HORIZONTAL:
      X = Paint.WidthMemory - X - 1;
      break;
    case MIRROR_VERTICAL:
      Y = Paint.HeightMemory - Y - 1;
      break;
    case MIRROR_ORIGIN:
      X = Paint.WidthMemory - X - 1;
      Y = Paint.HeightMemory - Y - 1;
      break;
    default:
      return;
  }

  // printf("x = %d, y = %d\r\n", X, Y);
  if (X > Paint.WidthMemory || Y > Paint.HeightMemory) {
    //Debug("Exceeding display boundaries\r\n");
    return;
  }
  // UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
  LCD_SetUWORD(X, Y, Color);
}

/******************************************************************************
  function: Clear the color of the picture
  parameter:
    Color   :   Painted colors
******************************************************************************/
void printBits(uint16_t num)
{
   for(int bit=0;bit<16; bit++)
   {
      printf("%i ", num & 0x01);
      num = num >> 1;
   }
   printf("\n");
}


void Paint_Clear(UWORD Color)
{
  // printBits(Color);
  LCD_SetCursor(0, 0, Paint.WidthByte , Paint.HeightByte);
  DEV_Digital_Write(DEV_DC_PIN, 1);
  // uint16_t swapped = ((Color & 0xFF00) >> 8) | ((Color & 0x00FF) << 8);
  // uint16_t buf[Paint.WidthByte*Paint.HeightByte] = {swapped};
  // spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  // spi_write16_blocking(spi0, buf, Paint.WidthByte*Paint.HeightByte);
  // spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  for (UWORD Y = 0; Y < Paint.HeightByte; Y++) {
    for (UWORD X = 0; X < Paint.WidthByte; X++ ) {//8 pixel =  1 byte
      LCD_WriteData_Word(Color);
    }
  }
}

/******************************************************************************
  function: Clear the color of a window
  parameter:
    Xstart :   x starting point
    Ystart :   Y starting point
    Xend   :   x end point
    Yend   :   y end point
******************************************************************************/
void Paint_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
{
  UWORD X, Y;
  for (Y = Ystart; Y < Yend; Y++) {
    for (X = Xstart; X < Xend; X++) {//8 pixel =  1 byte
      Paint_SetPixel(X, Y, Color);
    }
  }
}

/******************************************************************************
function:	Draw Point(Xpoint, Ypoint) Fill the color
parameter:
    Xpoint		:   The Xpoint coordinate of the point
    Ypoint		:   The Ypoint coordinate of the point
    Color		:   Set color
    Dot_Pixel	:	point size
******************************************************************************/
void Paint_DrawPoint( UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
    Paint_SetPixel(Xpoint, Ypoint, Color);
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
        Paint_DrawPoint(Xpoint, Ypoint, Color);
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
                        UWORD Color, DRAW_FILL Draw_Fill )
{
    //Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;

    //Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (Radius << 1 );

    int16_t sCountY;

    int xoffset;
    int yoffset;
    int offs;

    int x_boundary_l = std::max(X_Center-Radius-5, 0);
    int y_boundary_l = std::max(Y_Center-Radius-5, 0);
    int x_boundary_h = std::min(X_Center+Radius+5, (int)Paint.Width);
    int y_boundary_h = std::min(Y_Center+Radius+5, (int)Paint.Height);

    // printf("x_boundary_l ");
    // printf("%d", x_boundary_l);
    // printf("\n");

    // printf("y_boundary_l ");
    // printf("%d", y_boundary_l);
    // printf("\n");

    // printf("x_boundary_h ");
    // printf("%d", x_boundary_h);
    // printf("\n");

    // printf("y_boundary_h ");
    // printf("%d", y_boundary_h);
    // printf("\n");

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

    // if (Draw_Fill == DRAW_FILL_FULL) {
    //     while (XCurrent <= YCurrent ) { //Realistic circles
    //         for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
    //             Paint_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color);//1
    //             Paint_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color);//2
    //             Paint_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color);//3
    //             Paint_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color);//4
    //             Paint_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color);//5
    //             Paint_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color);//6
    //             Paint_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color);//7
    //             Paint_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color);
    //         }
    //         if (Esp < 0 )
    //             Esp += 4 * XCurrent + 6;
    //         else {
    //             Esp += 10 + 4 * (XCurrent - YCurrent );
    //             YCurrent --;
    //         }
    //         XCurrent ++;
    //     }
    // } else { //Draw a hollow circle
    //     while (XCurrent <= YCurrent ) {
    //         Paint_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color);//1
    //         Paint_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color);//2
    //         Paint_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color);//3
    //         Paint_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color);//4
    //         Paint_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color);//5
    //         Paint_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color);//6
    //         Paint_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color);//7
    //         Paint_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color);//0

    //         if (Esp < 0 )
    //             Esp += 4 * XCurrent + 6;
    //         else {
    //             Esp += 10 + 4 * (XCurrent - YCurrent );
    //             YCurrent --;
    //         }
    //         XCurrent ++;
    //     }
    // }
}

