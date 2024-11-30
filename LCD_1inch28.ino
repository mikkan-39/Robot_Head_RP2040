#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"

void SelectScreenR () {
  digitalWrite(DEV_CS_PIN, HIGH);
  digitalWrite(DEV_CS_PIN_2, LOW);
}

void SelectScreenL () {
  digitalWrite(DEV_CS_PIN, LOW);
  digitalWrite(DEV_CS_PIN_2, HIGH);
}

void SelectBothScreens () {
  digitalWrite(DEV_CS_PIN, LOW);
  digitalWrite(DEV_CS_PIN_2, LOW);
}

void setup()
{
  Serial.begin(9600);
  Config_Init();
  LCD_Init();
  SelectBothScreens();
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT+1, 0, BLACK);
  Paint_Clear(RED);

  SelectScreenL();
  Paint_DrawCircle(100, 150, 90, CYAN, DRAW_FILL_FULL);
  SelectScreenR();
  Paint_DrawCircle(140, 150, 90, CYAN, DRAW_FILL_FULL);

  // Paint_DrawCircle(100, 100, 40, CYAN, DRAW_FILL_FULL);
  // Paint_DrawPoint(100, 100, CYAN);
}

uint8_t x_start = 40;
uint8_t x_end = 200;

void loop()
{
  // SelectBothScreens();
  // SelectScreenR();
  // Paint_DrawRectangle(80, 100, 120, 140, CYAN, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
  // SelectScreenL();
  // Paint_DrawRectangle(100, 100, 140, 140, CYAN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  // for(uint8_t x = x_start; x < x_end; x++){
  //   // Paint_DrawLine(x, 100, x, 140, BLACK);
  //   // Paint_DrawLine(x+40, 100, x+40, 140, CYAN);
  //   Paint_DrawCircle(x-1, 100, 40, BLACK, DRAW_FILL_EMPTY);
  //   Paint_DrawCircle(x, 100, 40, CYAN, DRAW_FILL_FULL);
  // }
  // for(uint8_t x = x_end; x > x_start; x--){
  //   // Paint_DrawLine(x, 100, x, 140, CYAN);
  //   // Paint_DrawLine(x+40, 100, x+40, 140, BLACK);
  //   Paint_DrawCircle(x+1, 100, 40, BLACK, DRAW_FILL_EMPTY);
  //   Paint_DrawCircle(x, 100, 40, CYAN, DRAW_FILL_FULL);
  // }
  
  // Paint_DrawRectangle(120, 100, 160, 140, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  }


    
