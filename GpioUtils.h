#include "LCD_Driver.h"
#include "lcd.pio.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define SERIAL_CLK_DIV 1.f

void InitAllGpio();