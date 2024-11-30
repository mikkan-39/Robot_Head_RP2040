#include "DEV_Config.h"
#include "pico/stdlib.h"
#include "pico/"

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
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  // SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  spi_init(spi0)
  }

