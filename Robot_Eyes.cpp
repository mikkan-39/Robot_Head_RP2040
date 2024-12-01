#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "LCD_Driver.h"
#include "GUI_Paint.h"

#include "lcd.pio.h"

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


#define SERIAL_CLK_DIV 1.f

void DrawEye(UBYTE X, UBYTE Y, UBYTE Radius, UBYTE SideStep){
    uint8_t eye_offset = 50;
    SelectScreenR();
    Paint_DrawCircle(X, Y, Radius, CYAN, DRAW_FILL_FULL, SideStep);
    SelectScreenL();
    Paint_DrawCircle(X-eye_offset, Y, Radius, CYAN, DRAW_FILL_FULL, SideStep);
    SelectBothScreens();
}

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_puts(UART_ID, " Hello, UART!\n");
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    
    gpio_init(DEV_CS_PIN);
    gpio_init(DEV_CS_PIN_2);
    gpio_init(DEV_DC_PIN);
    gpio_init(DEV_RST_PIN);
    gpio_set_dir(DEV_CS_PIN, GPIO_OUT);
    gpio_set_dir(DEV_CS_PIN_2, GPIO_OUT);
    gpio_set_dir(DEV_DC_PIN, GPIO_OUT);
    gpio_set_dir(DEV_RST_PIN, GPIO_OUT);

    gpio_put(DEV_CS_PIN, 1);
    gpio_put(DEV_CS_PIN_2, 1);
    gpio_put(DEV_RST_PIN, 1);

    uint offset = pio_add_program(pio0, &lcd_program);
    lcd_program_init(pio0, pio_state_machine, offset, DEV_MOSI_PIN, DEV_SCK_PIN, SERIAL_CLK_DIV);

    SelectBothScreens();
    LCD_Init();
    Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, BLACK);
    
    SelectBothScreens();
    LCD_Clear(BLACK);

    uint8_t x_start = 90;
    uint8_t x_end = 180;
    uint8_t radius_start = 30;
    uint8_t radius_end = 120;
    uint8_t side_step = 4;
    uint8_t radius_step = 2;
    
    DRAW_FILL fill = DRAW_FILL_FULL;

    while (true) {
        // printf("Hello, world!\n");
        // sleep_ms(1000);
        for(uint8_t r = radius_start; r < radius_end; r+=radius_step){
            DrawEye(x_start, 150, r, side_step);
        }

        for(uint8_t x = x_start; x < x_end; x+=side_step){
            DrawEye(x, 150, radius_end, side_step);
        }

        for(uint8_t r = radius_end; r > radius_start; r-=radius_step){
            DrawEye(x_end, 150, r, side_step);
        }

        for(uint8_t x = x_end; x > x_start; x-=side_step){
            DrawEye(x, 150, radius_start, side_step);
        }
    }
}
