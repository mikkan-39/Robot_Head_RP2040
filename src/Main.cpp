#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <math.h>
#include <pico/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Core1.h"
#include "drivers/GpioUtils.h"
#include "gui/GUI_Paint.h"
#include "multicore/CommandParsers.h"
#include "multicore/MulticoreUtils.h"
// #define PICO_TIME_DEFAULT_ALARM_POOL_DISABLED 1

#include "lcd.pio.h"

// Adafruit_NXPSensorFusion filter3; // slowest
Adafruit_Madgwick filter; // faster than NXP
Adafruit_Mahony filter2;  // fastest/smalleset

float gx, gy, gz, ax, ay, az, mx, my, mz, vx, vy, vz, qx,
    qy, qz, qw;
float yaw, pitch, roll;

uint16_t TOFDistance = 0;

float sampleRate = 1000;

bool IMU_timer_callback(repeating_timer_t *rt) {
  accelerometer.readAccelerationGXYZ(ax, ay, az);
  gyroscope.readRotationDegXYZ(gx, gy, gz);
  compass.readCalibrateMagneticGaussXYZ(mx, my, mz);

  filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

  yaw = filter.getYaw();
  pitch = filter.getPitch();
  roll = filter.getRoll();

  filter.getGravityVector(&vx, &vy, &vz);
  filter.getQuaternion(&qw, &qx, &qy, &qz);

  return true;
}

int main() {
  stdio_init_all();

  InitAllGpio();

  compass.setRange(CompassRange::RANGE_4GAUSS);

  filter.begin(sampleRate);

  struct repeating_timer IMUtimer;
  struct repeating_timer TOFtimer;

  alarm_pool_t *alarm_pool =
      alarm_pool_create_with_unused_hardware_alarm(4);

  // alarm_pool_add_repeating_timer_us(
  //     alarm_pool,
  //     static_cast<int>(1000000 / sampleRate) * -1,
  //     IMU_timer_callback, NULL, &IMUtimer);

  while (true) {

    send_int_via_uart(compass.readX());
    send_string_via_uart(",");
    send_int_via_uart(compass.readY());
    send_string_via_uart(",");
    send_int_via_uart(compass.readZ());
    send_newline_via_uart();
    sleep_ms(100);

    // send_string_via_uart("READ_IMU: {");
    // send_string_via_uart("pitch: ");
    // send_float_via_uart(pitch);
    // send_string_via_uart(", roll: ");
    // send_float_via_uart(roll);
    // send_string_via_uart(", yaw: ");
    // send_float_via_uart(yaw);
    // // send_newline_via_uart();
    // // send_string_via_uart(", ax: ");
    // // send_float_via_uart(ax);
    // // send_string_via_uart(", ay: ");
    // // send_float_via_uart(ay);
    // // send_string_via_uart(", az: ");
    // // send_float_via_uart(az);
    // // send_string_via_uart(", gx: ");
    // // send_float_via_uart(gx);
    // // send_string_via_uart(", gy: ");
    // // send_float_via_uart(gy);
    // // send_string_via_uart(", gz: ");
    // // send_float_via_uart(gz);
    // send_string_via_uart(", mx: ");
    // send_float_via_uart(mx);
    // send_string_via_uart(", my: ");
    // send_float_via_uart(my);
    // send_string_via_uart(", mz: ");
    // send_float_via_uart(mz);
    // // send_string_via_uart(", sampleRate: ");
    // // send_float_via_uart(sampleRate);
    // send_string_via_uart("}");
    // send_newline_via_uart();
  }
}
