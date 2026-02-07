#include "pico/stdlib.h"
#include <stdio.h>
#include "ms5803_i2c/include/ms5803_i2c.h"
#include "dive_logger/include/dive_logger.h"
#include "ds3231.h"
#include "hardware/i2c.h"
#include "dive_display.h"
#include "dive_computer_pins.h"

#ifndef LED_DELAY_MS
    #define LED_DELAY_MS 1
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


float calc_depth_in_m(uint32_t pa, float water_density_kg_m3) {
    // Depth (m) = Pressure (Pa) / (Density (kg/m^3) * Gravity (m/s^2))
    uint32_t atmospheric_pressure_pa = 101325 + 200;
    int32_t gauge_pressure_pa = pa - atmospheric_pressure_pa;
    return ((float) gauge_pressure_pa) / (water_density_kg_m3 * 9.81f);
}

bool top_button_pressed() {
    return (gpio_get(TOP_BUTTON_PIN) == 0);
}

bool side_button_pressed() {
    return (gpio_get(SIDE_BUTTON_PIN) == 0);
}

int main() {
    stdio_init_all();

    // Setup Buttons
    gpio_init(TOP_BUTTON_PIN);
    gpio_set_dir(TOP_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(TOP_BUTTON_PIN);

    gpio_init(SIDE_BUTTON_PIN);
    gpio_set_dir(SIDE_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(SIDE_BUTTON_PIN);

    //sleep_ms(1000);
    //while(!side_button_pressed()) {}
    //printf("side pressed\n");

    //while(!top_button_pressed()) {}
    //printf("top pressed\n");

    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);

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
        .hour = 21,
        .minutes = 26,
        .seconds = 10,
        .dotw = 4
    };

    ds3231_set_datetime(&dt, &rtc);

    // Initialize MS5803 sensor (Used to get pressure and temperature)
    ms5803_inst_t* ms_inst = ms5803_init(ADC_4096);

    // Initialize dive log entry
    init_display();

    uint32_t pressure;

    log_entry.timestamp.year = 2025;
    log_entry.timestamp.month = 1;
    log_entry.timestamp.day = 1;
    log_entry.timestamp.hour = 0;
    log_entry.timestamp.min = 0;
    log_entry.timestamp.sec = 0;

    log_entry.temperature_c = 0.0f;
    log_entry.depth_m = 0.0f;

    // Start dive logging
    dive_logger_start("dive_log_04.bin");

    printf("entering main loop. SD Write successful\n");

    uint32_t tick = 0;

    while (tick < 60) {

        // Update pressure and temperature readings
        ms5803_update(ms_inst);

        log_entry.temperature_c = ms5803_get_temp(ms_inst, CELSIUS);

        pressure = ms5803_get_pressure(ms_inst);
        log_entry.depth_m = calc_depth_in_m(pressure, 1025.0f);

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
        display_dive_entry(&log_entry);

        // Wait till next entry
        printf("got temp %f C\n", log_entry.temperature_c);
        printf("Depth %f m\n", log_entry.depth_m);
        printf("got pressure %d pa\n\n\n", pressure);
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
