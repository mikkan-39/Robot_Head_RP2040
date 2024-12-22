#include "drivers/GpioUtils.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <cstdio>

Madgwick filter;

float gx, gy, gz, ax, ay, az;
float yaw, pitch, roll;

float sampleRate = 100;

void core1_thread() {
  //   printf("Core 1: Starting...\n");
  filter.begin();

  while (true) {
    unsigned long startTime = time_us_64();
    // Считываем данные с акселерометра в единицах G
    accelerometer.readAccelerationGXYZ(ax, ay, az);
    // Считываем данные с гироскопа в радианах в секунду
    gyroscope.readRotationRadXYZ(gx, gy, gz);
    // Устанавливаем частоту фильтра
    filter.setFrequency(sampleRate);
    // Обновляем входные данные в фильтр
    filter.update(gx, gy, gz, ax, ay, az);

    // Получаем из фильтра углы: yaw, pitch и roll
    yaw = filter.getYawDeg();
    pitch = filter.getPitchDeg();
    roll = filter.getRollDeg();

    // Read data from UART
    if (uart_is_readable(uart0)) {
      uint8_t uart_data = uart_getc(uart0);

      // Forward data to Core 0
      multicore_fifo_push_blocking(uart_data);
    }

    printf("%d", TOFsensor.readRangeSingleMillimeters());
    if (TOFsensor.timeoutOccurred()) {
      printf(" TIMEOUT");
    }
    printf("\n");

    unsigned long deltaTime = time_us_64() - startTime;
    sampleRate = 1000000 / deltaTime;
  }
}
