#include "GpioUtils.h"
#include "TOF_Driver.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define I2C_SDA 12
#define I2C_SCL 13

#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define SERIAL_CLK_DIV 1.f

void InitAllGpio() {
  // Set up our I2C
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Set up our UART
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_puts(UART_ID, " Hello, UART!\n");

  // GPIO
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

  // PIO
  uint offsetPioRight = pio_add_program(pio_instance_right, &lcd_program);
  lcd_program_init(pio_instance_right, pio_state_machine, offsetPioRight,
                   DEV_MOSI_PIN_RIGHT, DEV_SCK_PIN_RIGHT, SERIAL_CLK_DIV);

  uint offsetPioLeft = pio_add_program(pio_instance_left, &lcd_program);
  lcd_program_init(pio_instance_left, pio_state_machine, offsetPioLeft,
                   DEV_MOSI_PIN_LEFT, DEV_SCK_PIN_LEFT, SERIAL_CLK_DIV);

  if (!TOFsensor.init()) {
    printf("Failed to detect and initialize sensor!\n");
  }

  // lower the return signal rate limit (default is 0.25 MCPS)
  TOFsensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  TOFsensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  TOFsensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

  // increase timing budget to 200 ms
  TOFsensor.setMeasurementTimingBudget(20000);
}