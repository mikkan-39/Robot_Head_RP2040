#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "LCD_Driver.h"
#include "GUI_Paint.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 0
#define UART_RX_PIN 1



int main()
{
    stdio_init_all();
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    Config_Init();
    LCD_Init();
    SelectBothScreens();
    Paint_NewImage(LCD_WIDTH, LCD_HEIGHT+1, 0, BLACK);
    Paint_Clear(0x5555);

    // SelectScreenL();
    // Paint_DrawCircle(100, 150, 90, CYAN, DRAW_FILL_FULL);
    // SelectScreenR();
    // Paint_DrawCircle(140, 150, 90, CYAN, DRAW_FILL_FULL);

    SelectBothScreens();
    uint8_t x_start = 90;
    uint8_t x_end = 180;
    uint8_t radius_start = 30;
    uint8_t radius_end = 50;
    uint8_t side_step = 4;
    uint8_t radius_step = 2;
    uint8_t eye_offset = 50;
    DRAW_FILL fill = DRAW_FILL_FULL;
    while(true){
        Paint_Clear(0x5555);
        Paint_Clear(0x5555);
    }
    while (true) {
        // printf("Hello, world!\n");
        // sleep_ms(1000);
        // Paint_Clear(RED);
        // Paint_Clear(BLUE);
        for(uint8_t r = radius_start; r < radius_end; r+=radius_step){
            SelectScreenR();
            Paint_DrawCircle(x_start, 150, r, CYAN, fill);
            SelectScreenL();
            Paint_DrawCircle(x_start-eye_offset, 150, r, CYAN, fill);
        }

        for(uint8_t x = x_start; x < x_end; x+=side_step){
            SelectScreenR();
            Paint_DrawCircle(x, 150, radius_end, CYAN, fill);
            SelectScreenL();
            Paint_DrawCircle(x-eye_offset, 150, radius_end, CYAN, fill);
        }

        for(uint8_t r = radius_end; r > radius_start; r-=radius_step){
            SelectScreenR();
            Paint_DrawCircle(x_end, 150, r, CYAN, fill);
            SelectScreenL();
            Paint_DrawCircle(x_end-eye_offset, 150, r, CYAN, fill);
        }

        for(uint8_t x = x_end; x > x_start; x-=side_step){
            SelectScreenR();
            Paint_DrawCircle(x, 150, radius_start, CYAN, fill);
            SelectScreenL();
            Paint_DrawCircle(x-eye_offset, 150, radius_start, CYAN, fill);
        }
        // Paint_DrawRectangle(0, 0, 220, 220, RED, DRAW_FILL_FULL);
        // Paint_DrawRectangle(0, 0, 220, 220, BLUE, DRAW_FILL_FULL);
    }
}
