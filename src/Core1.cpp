#include "Core1.h"

Madgwick filter;

float gx, gy, gz, ax, ay, az;
float yaw, pitch, roll;

float sampleRate = 100;

void core1_thread() {
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
      char *uart_data = receive_command_from_uart();

      send_string_via_uart("ACK\n");
      send_string_via_uart("Forwarding to Core0\n");
      uart_puts(uart0, uart_data);
      // Forward data to Core 0
      send_string_to_core0(uart_data);
    }

    uint16_t TOFDistance =
        TOFsensor.readRangeSingleMillimeters();

    // send_uint16_via_uart(TOFDistance);

    // if (TOFsensor.timeoutOccurred()) {
    //   send_string_via_uart("TOF TIMEOUT");
    // }
    // send_newline_via_uart();

    unsigned long deltaTime = time_us_64() - startTime;
    sampleRate = 1000000 / deltaTime;
  }
}
