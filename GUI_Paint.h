#ifndef __GUI_PAINT_H
#define __GUI_PAINT_H

#include "LCD_Driver.h"
#include <cstring>

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define MAGENTA 0xF81F
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40
#define GRAY 0X8430
#define DARKBLUE 0X01CF
#define LIGHTBLUE 0X7D7C
#define GRAYBLUE 0X5458

enum ColorMap {
  BACKGROUND_COLOR = 0b00,
  PRIMARY_COLOR = 0b11,
  SECONDARY_COLOR = 0b01,
  RESERVED_COLOR = 0b10,
};

struct DISPLAY_BITMAP {
  static const uint8_t Height = LCD_HEIGHT;
  static const uint8_t Width = LCD_WIDTH;

  static const int _bitsPerColor = 2;
  static const int _pixelMask = 2 ^ _bitsPerColor - 1;
  static const int _pixelsPerByte = 8 / _bitsPerColor;

  uint16_t BackgroundColor; // 00
  uint16_t PrimaryColor;    // 11
  uint16_t SecondaryColor;  // 01
  uint16_t ReservedColor;   // 10

  uint8_t BitmapData[(Height * Width) / _pixelsPerByte]; // Packed 2-bit array

  // Fill the bitmap dimensions with background color
  void Initialize() { memset(BitmapData, 0b00000000, sizeof(BitmapData)); }

  // Set a pixel value (x, y) to a color index (0-3)
  void SetPixel(uint16_t x, uint16_t y, uint8_t colorIndex) {
    // if (x >= Width || y >= Height || colorIndex >= 2^_bitsPerColor) return;
    // // Out of bounds

    size_t pixelIndex = y * Width + x;
    size_t byteIndex = pixelIndex / _pixelsPerByte;
    size_t bitOffset = (pixelIndex % _pixelsPerByte) * _bitsPerColor;

    BitmapData[byteIndex] &= ~(_pixelMask << bitOffset);
    BitmapData[byteIndex] |= (colorIndex & _pixelMask) << bitOffset;
  }

  // Get a pixel value (x, y) as a color index (0-3)
  uint16_t GetPixel(uint16_t x, uint16_t y) const {
    // if (x >= Width || y >= Height) return 0; // Out of bounds

    size_t pixelIndex = y * Width + x;
    size_t byteIndex = pixelIndex / _pixelsPerByte;
    size_t bitOffset = (pixelIndex % _pixelsPerByte) * _bitsPerColor;

    uint8_t pixelBits = (BitmapData[byteIndex] >> bitOffset) & _pixelMask;
    switch (pixelBits) {
    case 0b11:
      return PrimaryColor;
    case 0b01:
      return SecondaryColor;
    case 0b10:
      return ReservedColor;
    default:
      return BackgroundColor;
    }
  }
};

extern DISPLAY_BITMAP BitmapRight;
extern DISPLAY_BITMAP BitmapLeft;

/**
 * Whether the graphic is filled
 **/
typedef enum {
  DRAW_FILL_EMPTY = 0,
  DRAW_FILL_FULL,
} DRAW_FILL;

static uint16_t ConvertBitmapPixelToColor(DISPLAY_BITMAP *bitmap, uint8_t bits);

void Bitmap_Init(DISPLAY_BITMAP *Bitmap, uint16_t PrimaryColor,
                 uint16_t SecondaryColor, uint16_t BackgroundColor);

void Bitmap_DrawLine(uint16_t Xstart, uint16_t Ystart, uint16_t Xend,
                     uint16_t Yend, uint16_t Color);
void Bitmap_DrawRectangle(uint16_t X_Center, uint16_t Y_Center, uint16_t Radius,
                          uint16_t Color, DRAW_FILL Filled);
void Bitmap_DrawCircle(uint16_t X_Center, uint16_t Y_Center, uint16_t Radius,
                       uint16_t Color, DRAW_FILL Draw_Fill);
void Bitmap_MoveEye(uint16_t Xstart, uint16_t Xend, uint16_t Ystart,
                    uint16_t Yend, uint16_t Rstart, uint16_t Rend,
                    uint16_t SideStep);

void BitmapsSend();

#endif
