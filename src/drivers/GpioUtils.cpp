#include "drivers/GpioUtils.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define BAUD_RATE    1000000
#define UART_TX_PIN  8
#define UART_RX_PIN  9

#define SERIAL_CLK_DIV 1.f

void InitAllGpio() {
  // UART
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Display GPIO
  gpio_init(DEV_CS_PIN_RIGHT);
  gpio_init(DEV_CS_PIN_LEFT);
  gpio_init(DEV_DC_PIN_RIGHT);
  gpio_init(DEV_DC_PIN_LEFT);
  gpio_init(DEV_RST_PIN);
  gpio_set_dir(DEV_CS_PIN_RIGHT, GPIO_OUT);
  gpio_set_dir(DEV_CS_PIN_LEFT,  GPIO_OUT);
  gpio_set_dir(DEV_DC_PIN_RIGHT, GPIO_OUT);
  gpio_set_dir(DEV_DC_PIN_LEFT,  GPIO_OUT);
  gpio_set_dir(DEV_RST_PIN,      GPIO_OUT);

  gpio_put(DEV_CS_PIN_RIGHT, 0);
  gpio_put(DEV_CS_PIN_LEFT,  0);
  gpio_put(DEV_RST_PIN,      1);

  // PIO (parallel SPI to both displays)
  uint offsetPioRight = pio_add_program(pio_instance_right, &lcd_program);
  lcd_program_init(pio_instance_right, pio_state_machine, offsetPioRight,
                   DEV_MOSI_PIN_RIGHT, DEV_SCK_PIN_RIGHT, SERIAL_CLK_DIV);

  uint offsetPioLeft = pio_add_program(pio_instance_left, &lcd_program);
  lcd_program_init(pio_instance_left, pio_state_machine, offsetPioLeft,
                   DEV_MOSI_PIN_LEFT, DEV_SCK_PIN_LEFT, SERIAL_CLK_DIV);
}
