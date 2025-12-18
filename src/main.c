#include "pico/stdlib.h"
#include <stdio.h>
#include "ms5803_i2c/include/ms5803_i2c.h"
#include "PicoTM1637.h"
#include "dive_logger/include/dive_logger.h"
#include "ds3231.h"
#include "hardware/i2c.h"

#define DS3231_I2C_PORT i2c_default
#define DS3231_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#define DS3231_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN


#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Perform initialisation
int pico_led_init(void) {
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
}

// Turn the led on or off
void pico_set_led(bool led_on) {
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
}

static dive_log_entry_t log_entry;

int main() {
    stdio_init_all();

    printf("setup ran!\n");

    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);

    // Initialize MS5803 sensor (Used to get pressure and temperature)
    ms5803_inst_t* ms_inst = ms5803_init(ADC_4096);

    TM1637_init(27, 26);
    TM1637_clear();
    TM1637_set_brightness(7);

    struct ds3231_rtc rtc;

    // Initialize DS3231 RTC
    ds3231_init(
        DS3231_I2C_PORT,
        DS3231_I2C_SDA_PIN,
        DS3231_I2C_SCL_PIN,
        &rtc
    );

    ds3231_datetime_t dt = {
        .year = 2025,
        .month = 12,
        .day = 18,
        .hour = 16,
        .minutes = 10,
        .seconds = 0,
        .dotw = 4
    };

    ds3231_set_datetime(&dt, &rtc);

    float temp;
    float pressure;
    float depth;

    log_entry.timestamp.year = 2025;
    log_entry.timestamp.month = 1;
    log_entry.timestamp.day = 1;
    log_entry.timestamp.hour = 0;
    log_entry.timestamp.min = 0;
    log_entry.timestamp.sec = 0;

    log_entry.temperature_c = 0.0f;
    log_entry.depth_m = 0.0f;

    int display_temp;
    int display_depth;

    // Start dive logging
    dive_logger_start("dive_log_04.bin");

    printf("entering main loop. SD Write successful\n");

    uint32_t tick = 0;

    float false_depth = 0.0f;

    while (tick < 30) {

        // Update pressure and temperature readings
        ms5803_update(ms_inst);

        log_entry.temperature_c = ms5803_get_temp(ms_inst, CELSIUS);

        pressure = ms5803_get_pressure(ms_inst);
        depth = (pressure * 0.033417) - 33;
        log_entry.depth_m = false_depth;
        false_depth += 0.5f;

        // Get current time from RTC
        ds3231_get_datetime(&dt, &rtc);



        // Update log entry
        log_entry.timestamp.year = dt.year;
        log_entry.timestamp.month = dt.month;
        log_entry.timestamp.day = dt.day;
        log_entry.timestamp.hour = dt.hour;
        log_entry.timestamp.min = dt.minutes;
        log_entry.timestamp.sec = dt.seconds;

        // Record log entry to SD Card
        dive_logger_record(&log_entry);



        // Display entry
        display_temp = (int) log_entry.temperature_c;
        display_depth = (int) log_entry.depth_m;

        TM1637_display((display_temp * 100) + display_depth, false);


        // Wait till next entry
        printf("got temp %f F\n", temp);
        printf("Depth %f\n", depth);
        printf("got pressure %f mbar\n\n\n", pressure);
        uint8_t dt_str[25];
        ds3231_ctime(dt_str, sizeof(dt_str), &dt);
        puts(dt_str);

        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);

        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);

        tick++;
    }

    // Stop dive logging
    dive_logger_stop();

    while (true) {
        sleep_ms(100);
    }
}
