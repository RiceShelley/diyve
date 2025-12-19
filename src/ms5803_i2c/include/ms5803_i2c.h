#ifndef MS5803_I2C__H
#define MS5803_I2C__H

#include "stdint.h"

// MS5803 I2C ADDR
#define MS5803_I2C_ADDR 0x76
#define MS5803_CONV_DELAY_MS 100

// Commands
#define CMD_RESET 0x1E // reset command 
#define CMD_ADC_READ 0x00 // ADC read command 
#define CMD_ADC_CONV 0x40 // ADC conversion command 

#define CMD_PROM 0xA0 // Coefficient location

// Define units for conversions. 
typedef enum {
	CELSIUS,
	FAHRENHEIT,
} temperature_units;

// Define measurement type.
typedef enum {	
	PRESSURE = 0x00,
	TEMPERATURE = 0x10
} measurement;

// Define constants for Conversion precision
typedef enum {
	ADC_256  = 0x00,
	ADC_512  = 0x02,
	ADC_1024 = 0x04,
	ADC_2048 = 0x06,
	ADC_4096 = 0x08
} precision;

typedef struct ms5803_inst ms5803_inst_t;

ms5803_inst_t* ms5803_init(precision p);
void ms5803_update(ms5803_inst_t *inst);
float ms5803_get_temp(ms5803_inst_t *inst, temperature_units units);
uint32_t ms5803_get_pressure(ms5803_inst_t *inst);

#endif
