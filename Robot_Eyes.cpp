#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <math.h>
#include <pico/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd.pio.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define SERIAL_CLK_DIV 4.f

uint16_t get_rand_16() { return get_rand_32() & 0x0000ffff; }

int restrainedCoords(int coords) { return std::min(std::max(coords, 0), 239); }

int main() {
  stdio_init_all();

  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  // For more examples of I2C use see
  // https://github.com/raspberrypi/pico-examples/tree/master/i2c

  // Set up our UART
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_puts(UART_ID, " Hello, UART!\n");
  // For more examples of UART use see
  // https://github.com/raspberrypi/pico-examples/tree/master/uart

  // while (true) {
  //     uart_puts(UART_ID, "Somebody once told me\n");
  // }

  gpio_init(DEV_CS_PIN_RIGHT);
  gpio_init(DEV_CS_PIN_LEFT);
  gpio_init(DEV_DC_PIN_RIGHT);
  gpio_init(DEV_DC_PIN_LEFT);
  gpio_init(DEV_RST_PIN);
  gpio_set_dir(DEV_CS_PIN_RIGHT, GPIO_OUT);
  gpio_set_dir(DEV_CS_PIN_LEFT, GPIO_OUT);
  gpio_set_dir(DEV_DC_PIN_RIGHT, GPIO_OUT);
  gpio_set_dir(DEV_DC_PIN_LEFT, GPIO_OUT);
  gpio_set_dir(DEV_RST_PIN, GPIO_OUT);

  gpio_put(DEV_CS_PIN_RIGHT, 0);
  gpio_put(DEV_CS_PIN_LEFT, 0);
  gpio_put(DEV_RST_PIN, 1);

  uint offsetPioRight = pio_add_program(pio_instance_right, &lcd_program);
  lcd_program_init(pio_instance_right, pio_state_machine, offsetPioRight,
                   DEV_MOSI_PIN_RIGHT, DEV_SCK_PIN_RIGHT, SERIAL_CLK_DIV);

  uint offsetPioLeft = pio_add_program(pio_instance_left, &lcd_program);
  lcd_program_init(pio_instance_left, pio_state_machine, offsetPioLeft,
                   DEV_MOSI_PIN_LEFT, DEV_SCK_PIN_LEFT, SERIAL_CLK_DIV);

  Bitmap_Init(&BitmapRight, CYAN, MAGENTA, BLACK);
  Bitmap_Init(&BitmapLeft, MAGENTA, CYAN, BLACK);

  LCD_Both_Init();
  BitmapsSend();

  float prevX = 120;
  float prevY = 120;
  float prevR = 40;
  float seed;
  float nextX;
  float nextY;
  float nextR;

  while (true) {
    BitmapRight.BackgroundColor = CYAN;
    BitmapLeft.BackgroundColor = CYAN;
    BitmapRight.UpdateColorLookup();
    BitmapLeft.UpdateColorLookup();
    BitmapsSend();

    BitmapRight.BackgroundColor = BLACK;
    BitmapLeft.BackgroundColor = BLACK;
    BitmapRight.UpdateColorLookup();
    BitmapLeft.UpdateColorLookup();
    BitmapsSend();
  }

  while (true) {
    BitmapRight.PrimaryColor = CYAN;
    BitmapLeft.PrimaryColor = CYAN;
    BitmapRight.UpdateColorLookup();
    BitmapLeft.UpdateColorLookup();

    seed = (float)get_rand_16() / (float)0xffff;
    nextX = 120 - 50 + 100 * seed;
    seed = (float)get_rand_16() / (float)0xffff;
    nextY = 120 - 50 + 100 * seed;
    seed = (float)get_rand_16() / (float)0xffff;
    nextR = 30 + (90 - 30) * seed;

    Bitmap_MoveEye(restrainedCoords((int)prevX), restrainedCoords((int)nextX),
                   restrainedCoords((int)prevY), restrainedCoords((int)nextY),
                   (int)prevR, (int)nextR, 5);

    prevX = nextX;
    prevY = nextY;
    prevR = nextR;

    sleep_ms(get_rand_32() & 0x2ff);

    int jiggleCount = (get_rand_32() & 0x3) + 2;

    BitmapRight.PrimaryColor = MAGENTA;
    BitmapLeft.PrimaryColor = MAGENTA;
    BitmapRight.UpdateColorLookup();
    BitmapLeft.UpdateColorLookup();

    float localX = prevX;
    float localY = prevY;
    for (int i = 0; i < jiggleCount; i++) {
      nextX = (float)((int)prevX + (get_rand_32() & 0x0f) - 7);
      nextY = (float)((int)prevY + (get_rand_32() & 0x0f) - 7);

      Bitmap_MoveEye(restrainedCoords((int)localX),
                     restrainedCoords((int)nextX),
                     restrainedCoords((int)localY),
                     restrainedCoords((int)nextY), (int)prevR, (int)prevR, 1);

      localX = nextX;
      localY = nextY;
      prevR = nextR;

      sleep_ms(get_rand_32() & 0x2ff);
    }
    prevX = localX;
    prevY = localY;
  }
}
