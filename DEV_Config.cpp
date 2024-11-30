#include "DEV_Config.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  2
#define PIN_MOSI 3

void GPIO_Init()
{
  gpio_init(DEV_CS_PIN); 
  gpio_init(DEV_CS_PIN_2);  
  gpio_init(DEV_RST_PIN);   
  gpio_init(DEV_DC_PIN);

  gpio_set_dir(DEV_CS_PIN, GPIO_OUT);
  gpio_set_dir(DEV_CS_PIN_2, GPIO_OUT);
  gpio_set_dir(DEV_RST_PIN, GPIO_OUT);
  gpio_set_dir(DEV_DC_PIN, GPIO_OUT);
 }

 void Config_Init()
 {

  GPIO_Init();

  //spi
    spi_init(SPI_PORT, 1000*1000*62.5);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_dir(PIN_CS, GPIO_OUT);
  }

