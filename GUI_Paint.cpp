#include "GUI_Paint.h"
#include "hardware/spi.h"
#include "lcd.pio.h"
#include "pico/malloc.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

DISPLAY_BITMAP BitmapRight;
DISPLAY_BITMAP BitmapLeft;

/******************************************************************************
  function: Create Image
  parameter:
    Bitmap          :   Pointer to the bitmap
    Width           :   The width of the picture
    Height          :   The height of the picture
    PrimaryColor    :   Eye color
    SecondaryColor  :   Eyelids etc color
    BackgroundColor :   BG color
******************************************************************************/
void Bitmap_Init(DISPLAY_BITMAP *Bitmap, uint16_t PrimaryColor,
                 uint16_t SecondaryColor, uint16_t BackgroundColor) {
  Bitmap->PrimaryColor = PrimaryColor;
  Bitmap->SecondaryColor = SecondaryColor;
  Bitmap->BackgroundColor = BackgroundColor;
  Bitmap->UpdateColorLookup();
  Bitmap->Clear();
}

// function:	Draw a line of arbitrary slope
void Bitmap_DrawLine(DISPLAY_BITMAP *Bitmap, uint16_t Xstart, uint16_t Ystart,
                     uint16_t Xend, uint16_t Yend, ColorMap Color) {
  uint16_t Xpoint = Xstart;
  uint16_t Ypoint = Ystart;

  int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

  // Increment direction, 1 is positive, -1 is counter;
  int XAddway = Xstart < Xend ? 1 : -1;
  int YAddway = Ystart < Yend ? 1 : -1;

  // Cumulative error
  int Esp = dx + dy;

  for (;;) {
    Bitmap->SetPixel(Xpoint, Ypoint, Color);
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

// function:	Draw a rectangle
void Bitmap_DrawRectangle(DISPLAY_BITMAP *Bitmap, uint16_t X_Center,
                          uint16_t Y_Center, uint16_t Radius, ColorMap Color,
                          DRAW_FILL Filled) {
  int Xstart = std::max(X_Center - Radius, 0);
  int Ystart = std::max(Y_Center - Radius, 0);
  int Xend = std::min(X_Center + Radius, (int)Bitmap->Width - 1);
  int Yend = std::min(Y_Center + Radius, (int)Bitmap->Height - 1);

  if (Filled) {
    for (int y = Ystart; y <= Yend; y++) {
      for (int x = Xstart; x <= Xend; x++) {
        Bitmap->SetPixel(x, y, Color);
      }
    }
  } else {
    Bitmap_DrawLine(Xstart, Ystart, Xend, Ystart, Color);
    Bitmap_DrawLine(Xstart, Ystart, Xstart, Yend, Color);
    Bitmap_DrawLine(Xend, Yend, Xend, Ystart, Color);
    Bitmap_DrawLine(Xend, Yend, Xstart, Yend, Color);
  }
}

// function:	Draw a circle of the specified size at the specified position
void Bitmap_DrawCircle(DISPLAY_BITMAP *Bitmap, uint16_t X_Center,
                       uint16_t Y_Center, uint16_t Radius, ColorMap Color,
                       DRAW_FILL Draw_Fill) {
  int xoffset;
  int yoffset;
  int offs;

  int x_boundary_l = std::max((int)X_Center - (int)Radius, 0);
  int y_boundary_l = std::max((int)Y_Center - (int)Radius, 0);
  int x_boundary_h =
      std::min((int)X_Center + (int)Radius, (int)Bitmap->Width - 1);
  int y_boundary_h =
      std::min((int)Y_Center + (int)Radius, (int)Bitmap->Height - 1);

  if (Draw_Fill == DRAW_FILL_FULL) {
    for (int y = y_boundary_l; y <= y_boundary_h; y++) {
      for (int x = x_boundary_l; x <= x_boundary_h; x++) {
        xoffset = X_Center - x;
        yoffset = Y_Center - y;
        if (xoffset * xoffset + yoffset * yoffset <= Radius * Radius) {
          Bitmap->SetPixel(x, y, Color);
        }
      }
    }
  } else {
    for (int y = y_boundary_l; y <= y_boundary_h; y++) {
      for (int x = x_boundary_l; x <= x_boundary_h; x++) {
        xoffset = X_Center - x;
        yoffset = Y_Center - y;
        offs = xoffset * xoffset + yoffset * yoffset - Radius * Radius;
        if (offs <= 0) {
          if (offs > -Radius * 10) {
            Bitmap->SetPixel(x, y, Color);
          }
        }
      }
    }
  }
}

void DrawEye(uint8_t X, uint8_t Y, uint8_t Radius) {
  uint8_t r_eye_offset = 15;
  uint8_t l_eye_offset = 15;
  BitmapRight.Clear();
  Bitmap_DrawCircle(&BitmapRight, X + r_eye_offset, Y, Radius, PRIMARY_COLOR,
                    DRAW_FILL_FULL);
  BitmapLeft.Clear();
  Bitmap_DrawCircle(&BitmapLeft, X - l_eye_offset, Y, Radius, PRIMARY_COLOR,
                    DRAW_FILL_FULL);
  BitmapsSend();
}

void Bitmap_MoveEye(uint16_t Xstart, uint16_t Xend, uint16_t Ystart,
                    uint16_t Yend, uint16_t Rstart, uint16_t Rend,
                    uint16_t SideStep) {
  uint16_t Xpoint = Xstart;
  uint16_t Ypoint = Ystart;
  uint16_t Rpoint = Rstart;

  int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  int dy = (int)Yend - (int)Ystart >= 0 ? Yend - Ystart : Ystart - Yend;
  int dr = (int)Rend - (int)Rstart >= 0 ? Rend - Rstart : Rstart - Rend;
  int dmax = std::max(std::max(dx, dy), dr);
  int steps = dmax / SideStep;

  if (dmax == 0 || steps == 0) {
    DrawEye(Xpoint, Ypoint, Rpoint);
    return;
  }

  for (int i = 0; i < steps; i++) {
    DrawEye(Xpoint, Ypoint, Rpoint);
    float progress = (float)i / (float)steps;
    float Xprogress = (Xend - Xstart) * progress;
    float Yprogress = (Yend - Ystart) * progress;
    float Rprogress = (Rend - Rstart) * progress;

    Xpoint = Xstart + (int)Xprogress;
    Ypoint = Ystart + (int)Yprogress;
    Rpoint = Rstart + (int)Rprogress;
  }

  if (Rpoint != Rend || Xpoint != Xend || Ypoint != Yend)
    DrawEye(Xpoint, Ypoint, Rpoint);
}

// fucntion: Convert both bitmaps to colors and send
void BitmapsSend() {
  LCD_Both_SetCursor(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
  for (int byteIndex = 0; byteIndex < sizeof(BitmapRight.BitmapData);
       byteIndex++) {
    uint8_t currentByteR = BitmapRight.BitmapData[byteIndex];
    uint8_t currentByteL = BitmapLeft.BitmapData[byteIndex];

    // Convert and write all pixels

    // Extract bits 0-1
    uint32_t pixel0 = BitmapRight.colorLookup[currentByteR & 0x03];
    uint32_t pixel4 = BitmapLeft.colorLookup[currentByteL & 0x03];

    // Extract bits 2-3
    uint32_t pixel1 = BitmapRight.colorLookup[(currentByteR >> 2) & 0x03];
    uint32_t pixel5 = BitmapLeft.colorLookup[(currentByteL >> 2) & 0x03];

    lcd_wait_async(pio_instance_right, pio_state_machine);
    lcd_wait_async(pio_instance_left, pio_state_machine);
    lcd_put32_async(pio_instance_right, pio_state_machine, pixel0 << 16 | pixel1);
    lcd_put32_async(pio_instance_left, pio_state_machine, pixel4 << 16 | pixel5);
    
    // Extract bits 4-5
    uint32_t pixel2 = BitmapRight.colorLookup[(currentByteR >> 4) & 0x03];
    uint32_t pixel6 = BitmapLeft.colorLookup[(currentByteL >> 4) & 0x03];

    // Extract bits 6-7
    uint32_t pixel3 = BitmapRight.colorLookup[(currentByteR >> 6) & 0x03];
    uint32_t pixel7 = BitmapLeft.colorLookup[(currentByteL >> 6) & 0x03];

    lcd_wait_async(pio_instance_right, pio_state_machine);
    lcd_wait_async(pio_instance_left, pio_state_machine);
    lcd_put32_async(pio_instance_right, pio_state_machine, pixel2 << 16 | pixel3);
    lcd_put32_async(pio_instance_left, pio_state_machine, pixel6 << 16 | pixel7);
  }
}