#include "pico/stdlib.h"
#include <stdio.h>
#include "ms5803_i2c/include/ms5803_i2c.h"
#include "PicoTM1637.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Perform initialisation
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

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

    int count = 0;

    float temp;
    float pressure;
    float depth;

    int display_temp;
    int display_depth;

    while (true) {
        ms5803_update(ms_inst);

        temp = ms5803_get_temp(ms_inst, FAHRENHEIT);
        pressure = ms5803_get_pressure(ms_inst);

        depth = (pressure * 0.033417) - 33;

        display_temp = (int) temp;
        display_depth = (int) depth;

        TM1637_display((display_temp * 100) + display_depth, false);

        printf("got temp %f F\n", temp);
        printf("Depth %f\n", depth);
        printf("got pressure %f mbar\n\n\n", pressure);

        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);

        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);
    }
}
