#include "include/ms5803_i2c.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "dive_computer_pins.h"
#include <stdint.h>

struct ms5803_inst {
    precision p;

    uint16_t coefficient[8];

    int32_t temp_raw;
    int32_t pressure_raw;
};

static ms5803_inst_t ms5803_state;

ms5803_inst_t* ms5803_init(precision p) {

    i2c_init(MS5803_I2C_PORT, 10 * 1000);

    gpio_set_function(MS5803_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MS5803_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MS5803_I2C_SDA_PIN);
    gpio_pull_up(MS5803_I2C_SCL_PIN);

    // Set precision
    ms5803_state.p = p;

    // Read coefficients
    int i = 0;
    uint8_t coef_buff[2];
    uint8_t txdata = CMD_RESET;
    uint8_t rxdata;
    int ret;
    for (i = 0; i < 8; i++) {
        txdata = CMD_PROM + i * 2;
        ret = i2c_write_blocking(MS5803_I2C_PORT, MS5803_I2C_ADDR, &txdata, 1, false);
        ret = i2c_read_blocking(MS5803_I2C_PORT, MS5803_I2C_ADDR, &coef_buff[0], 2, false);
        ms5803_state.coefficient[i] = ((uint16_t) coef_buff[0] << 8) | ((uint16_t) coef_buff[1]);
    }

    return &ms5803_state;
}

uint32_t getADCconversion(measurement m, precision p) {
    uint8_t txdata;
    uint8_t rxdata[3];
    int ret;
    uint32_t result;

    // SEND ADC Conversion CMD
    txdata = CMD_ADC_CONV | m | p;
    ret = i2c_write_blocking(MS5803_I2C_PORT, MS5803_I2C_ADDR, &txdata, 1, false);

    // Wait for Conversion to complete
    sleep_ms(MS5803_CONV_DELAY_MS);

    // Send ADC read CMD
    txdata = CMD_ADC_READ;
    ret = i2c_write_blocking(MS5803_I2C_PORT, MS5803_I2C_ADDR, &txdata, 1, false);

    // Read result
    ret = i2c_read_blocking(MS5803_I2C_PORT, MS5803_I2C_ADDR, rxdata, 3, false);

    result = ((uint32_t) rxdata[0] << 16) | ((uint32_t) rxdata[1] << 8) | ((uint32_t) rxdata[2]);

    return result;
}

void ms5803_update(ms5803_inst_t *inst) {
	// Retrieve ADC result
	int32_t temperature_raw = getADCconversion(TEMPERATURE, inst->p);
	int32_t pressure_raw = getADCconversion(PRESSURE, inst->p);
	
	// Create Variables for calculations
	int32_t temp_calc;
	int32_t pressure_calc;
	
	int32_t dT;
		
	// Now that we have a raw temperature, let's compute our actual.
	dT = temperature_raw - ((int32_t) inst->coefficient[5] << 8);
	temp_calc = (((int64_t)dT * inst->coefficient[6]) >> 23) + 2000;
	
	// TODO TESTING  _temperature_actual = temp_calc;
	
	//Now we have our first order Temperature, let's calculate the second order.
	int64_t T2, OFF2, SENS2, OFF, SENS; //working variables

	if (temp_calc < 2000) 
	// If temp_calc is below 20.0C
	{	
		T2 = 3 * (((int64_t)dT * dT) >> 33);
		OFF2 = 3 * ((temp_calc - 2000) * (temp_calc - 2000)) / 2;
		SENS2 = 5 * ((temp_calc - 2000) * (temp_calc - 2000)) / 8;
		
		if(temp_calc < -1500)
		// If temp_calc is below -15.0C 
		{
			OFF2 = OFF2 + 7 * ((temp_calc + 1500) * (temp_calc + 1500));
			SENS2 = SENS2 + 4 * ((temp_calc + 1500) * (temp_calc + 1500));
		}
    } 
	else
	// If temp_calc is above 20.0C
	{ 
		//T2 = 7 * ((uint64_t)dT * dT)/pow(2,37);
		T2 = 7 * ((uint64_t)dT * dT)/ (((uint64_t) 1) << 37);
		OFF2 = ((temp_calc - 2000) * (temp_calc - 2000)) / 16;
		SENS2 = 0;
	}
	
	// Now bring it all together to apply offsets 
	
	OFF = ((int64_t)inst->coefficient[2] << 16) + (((inst->coefficient[4] * (int64_t)dT)) >> 7);
	SENS = ((int64_t)inst->coefficient[1] << 15) + (((inst->coefficient[3] * (int64_t)dT)) >> 8);
	
	temp_calc = temp_calc - T2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;

	// Now lets calculate the pressure
	pressure_calc = (((SENS * pressure_raw) / 2097152 ) - OFF) / 32768;

    inst->temp_raw = temp_calc;
    inst->pressure_raw = pressure_calc;
}

float ms5803_get_temp(ms5803_inst_t *inst, temperature_units units) {
    // Return a temperature reading in either F or C.

    // Convert temperature
	float temperature_reported;

	if (units == FAHRENHEIT) {
	    // If Fahrenheit is selected return the temperature converted to F
		temperature_reported = inst->temp_raw / 100;
		temperature_reported = (((temperature_reported) * 9) / 5) + 32;
		return temperature_reported;
	} else {
	    // If Celsius is selected return the temperature converted to C	
		temperature_reported = inst->temp_raw / 100;
		return temperature_reported;
	}
}
#include <stdio.h>
uint32_t ms5803_get_pressure(ms5803_inst_t *inst) {
    // Return a pressure reading in pascals
	return inst->pressure_raw * 10;
}
