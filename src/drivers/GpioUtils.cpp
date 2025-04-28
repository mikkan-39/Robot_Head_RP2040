#include "drivers/GpioUtils.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define I2C_SDA 12
#define I2C_SCL 13

#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define SERIAL_CLK_DIV 1.f

Gyroscope gyroscope;
Accelerometer accelerometer;
Compass compass;

const float compassCalibrationBias[3] = {0.0, 0.0, 0.0};

const float compassCalibrationMatrix[3][3] = {
    {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

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

  gyroscope.begin();
  accelerometer.begin();
  compass.begin();
  // compass.setCalibrateMatrix(compassCalibrationMatrix,
  //                            compassCalibrationBias);
}