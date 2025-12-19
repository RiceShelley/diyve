#ifndef DIVE_COMPUTER_PINS__H
#define DIVE_COMPUTER_PINS__H

#include "hardware/i2c.h"


#define DS3231_I2C_PORT i2c0
#define DS3231_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN
#define DS3231_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN

#define MS5803_I2C_PORT i2c1
#define MS5803_I2C_SCL_PIN 27
#define MS5803_I2C_SDA_PIN 26

#define SD_SPI_PORT spi0
#define SD_SPI_MISO_PIN 16
#define SD_SPI_MOSI_PIN 19
#define SD_SPI_SCK_PIN 18
#define SD_SPI_SS_PIN 17

#define EPD_SPI_PORT spi1
#define EPD_DC_PIN 6
#define EPD_RST_PIN 7
#define EPD_BUSY_PIN 8
#define EPD_CS_PIN 13
#define EPD_CLK_PIN 10
#define EPD_MOSI_PIN 11

#endif