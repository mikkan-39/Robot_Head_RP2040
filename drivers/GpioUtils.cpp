#include "GpioUtils.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

void InitAllGpio() {
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
}