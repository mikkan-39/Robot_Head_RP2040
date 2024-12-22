#include "LCD_Driver.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "lcd.pio.h"
#include "pico/stdlib.h"

void lcd_set_dc_cs(bool dc, bool cs) {
  lcd_wait_idle(pio_instance_right, pio_state_machine);
  lcd_wait_idle(pio_instance_left, pio_state_machine);

  // gpio_put_masked((1u << DEV_DC_PIN_RIGHT) | (1u << DEV_DC_PIN_LEFT) |
  //                     (1u << DEV_CS_PIN_RIGHT) | (1u << DEV_CS_PIN_LEFT),
  //                 !!dc << DEV_DC_PIN_RIGHT | !!dc << DEV_DC_PIN_LEFT |
  //                     !!cs << DEV_CS_PIN_RIGHT | !!cs << DEV_CS_PIN_LEFT);

  gpio_put_masked((1u << DEV_DC_PIN_RIGHT) | (1u << DEV_DC_PIN_LEFT),
                  !!dc << DEV_DC_PIN_RIGHT | !!dc << DEV_DC_PIN_LEFT);
}

void LCD_Reset(void) {
  sleep_ms(100);
  gpio_put(DEV_RST_PIN, 0);
  sleep_ms(100);
  gpio_put(DEV_RST_PIN, 1);
  sleep_ms(100);
}

void LCD_WriteData_Byte(PIO pio_instance, uint8_t da) {
  lcd_put(pio_instance, pio_state_machine, da);
  lcd_wait_idle(pio_instance, pio_state_machine);
}

void LCD_WriteData_Byte_Both(uint8_t da) {
  lcd_put(pio_instance_right, pio_state_machine, da);
  lcd_put(pio_instance_left, pio_state_machine, da);
  lcd_wait_idle(pio_instance_right, pio_state_machine);
  lcd_wait_idle(pio_instance_left, pio_state_machine);
}

void LCD_WriteData_Word(PIO pio_instance, uint16_t da) {
  lcd_put(pio_instance, pio_state_machine, da >> 8);
  lcd_put(pio_instance, pio_state_machine, da & 0xff);
}

void LCD_Both_WriteReg(uint8_t da) {
  lcd_wait_idle(pio_instance_right, pio_state_machine);
  lcd_wait_idle(pio_instance_left, pio_state_machine);

  gpio_put(DEV_DC_PIN_RIGHT, 0);
  gpio_put(DEV_DC_PIN_LEFT, 0);

  lcd_put(pio_instance_right, pio_state_machine, da);
  lcd_put(pio_instance_left, pio_state_machine, da);

  lcd_wait_idle(pio_instance_right, pio_state_machine);
  lcd_wait_idle(pio_instance_left, pio_state_machine);

  gpio_put(DEV_DC_PIN_RIGHT, 1);
  gpio_put(DEV_DC_PIN_LEFT, 1);
}

void lcd_write_both_cmd(const uint8_t *cmd, size_t count) {
  lcd_wait_idle(pio_instance_right, pio_state_machine);
  lcd_wait_idle(pio_instance_left, pio_state_machine);

  lcd_set_dc_cs(0, 0);

  uint8_t next = *cmd++;
  lcd_put(pio_instance_right, pio_state_machine, next);
  lcd_put(pio_instance_left, pio_state_machine, next);

  if (count >= 2) {
    lcd_wait_idle(pio_instance_right, pio_state_machine);
    lcd_wait_idle(pio_instance_left, pio_state_machine);

    lcd_set_dc_cs(1, 0);
    for (size_t i = 0; i < count - 1; ++i) {
      uint8_t next = *cmd++;
      lcd_put(pio_instance_right, pio_state_machine, next);
      lcd_put(pio_instance_left, pio_state_machine, next);
    }
  }

  lcd_wait_idle(pio_instance_right, pio_state_machine);
  lcd_wait_idle(pio_instance_left, pio_state_machine);

  lcd_set_dc_cs(1, 1);
}

void lcd_pipe_commands(const uint8_t *init_seq) {
  const uint8_t *cmd = init_seq;
  while (*cmd) {
    lcd_write_both_cmd(cmd + 2, *cmd);
    sleep_ms(*(cmd + 1) * 5);
    cmd += *cmd + 2;
  }
}

/******************************************************************************
function:
    Common register initialization
******************************************************************************/
void LCD_Both_Init(void) {
  LCD_Reset();

  // clang-format off
  const uint8_t init_seq[] = {
    1, 1, 0xEF,                       
    2, 1, 0xEB, 0x14,                  
    1, 1, 0xFE, 
    1, 1, 0xEF,
    2, 1, 0xEB, 0x14,
    2, 1, 0x84, 0x40,
    2, 1, 0x85, 0xFF,
    2, 1, 0x86, 0xFF,
    2, 1, 0x87, 0xFF,
    2, 1, 0x88, 0x0A,
    2, 1, 0x89, 0x21,
    2, 1, 0x8A, 0x00,
    2, 1, 0x8B, 0x80,
    2, 1, 0x8C, 0x01,
    2, 1, 0x8D, 0x01,
    2, 1, 0x8E, 0xFF,
    2, 1, 0x8F, 0xFF,
    3, 1, 0xB6, 0x00, 0x20,
    2, 1, 0x36, 0x08,
    2, 1, 0x3A, 0x05,
    6, 1, 0x90, 0x08, 0x08, 0x08, 0x08,
    2, 1, 0xBD, 0x06,
    2, 1, 0xBC, 0x00,
    4, 1, 0xFF, 0x60, 0x01, 0x04,
    2, 1, 0xC3, 0x13,
    2, 1, 0xC4, 0x13,
    2, 1, 0xC9, 0x22,
    2, 1, 0xBE, 0x11,
    3, 1, 0xE1, 0x10, 0x0E,
    4, 1, 0xDF, 0x21, 0x0C, 0x02,
    7, 1, 0xF0, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
    7, 1, 0xF1, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
    7, 1, 0xF2, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
    7, 1, 0xF3, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
    3, 1, 0xED, 0x1B, 0x0B,
    2, 1, 0xAE, 0x77,
    2, 1, 0xCD, 0x63,
    10, 1, 0x70, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03,
    2, 1, 0xE8, 0x34,
    13, 1, 0x62, 0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70,
    13, 1, 0x63, 0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70,
    8, 1, 0x64, 0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07,
    11, 1, 0x66, 0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00,
    11, 1, 0x67, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98,
    8, 1, 0x74, 0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00,
    3, 1, 0x98, 0x3E, 0x07,
    1, 1, 0x35,
    1, 1, 0x21,
    1, 50, 0x11,
    1, 5, 0x29,          
    0                                   
  };
  // clang-format on

  lcd_pipe_commands(init_seq);
  LCD_Both_SetCursor(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

// function: Set the current draw box
void LCD_Both_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend,
                        uint16_t Yend) {
  LCD_Both_WriteReg(0x2a);
  LCD_WriteData_Byte_Both(0x00);
  LCD_WriteData_Byte_Both(Xstart);
  LCD_WriteData_Byte_Both(0x00);
  LCD_WriteData_Byte_Both(Xend);

  LCD_Both_WriteReg(0x2b);
  LCD_WriteData_Byte_Both(0x00);
  LCD_WriteData_Byte_Both(Ystart);
  LCD_WriteData_Byte_Both(0x00);
  LCD_WriteData_Byte_Both(Yend);

  LCD_Both_WriteReg(0x2c);
}
