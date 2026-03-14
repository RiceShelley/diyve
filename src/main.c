#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "ms5803_i2c/include/ms5803_i2c.h"
#include "ds3231.h"
#include "hardware/i2c.h"
#include "dive_display.h"
#include "diyve_pins.h"
#include "gui.h"

#define RTC_SYNC_TIMEOUT_MS 2000

// Listen on UART for a TIME:YYYY-MM-DDTHH:MM:SS:DOW packet and set the RTC.
// Waits up to RTC_SYNC_TIMEOUT_MS then continues regardless.
static void sync_rtc_from_uart(struct ds3231_rtc *rtc) {
    // Repeatedly signal the host until we get a response or time out.
    // Sending repeatedly ensures the host catches it even if it starts late.
    char buf[32];
    int idx = 0;
    uint32_t start = to_ms_since_boot(get_absolute_time());
    uint32_t last_prompt = 0;

    while (to_ms_since_boot(get_absolute_time()) - start < RTC_SYNC_TIMEOUT_MS) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_prompt >= 200) {
            printf("SYNC?\n");
            last_prompt = now;
        }

        int c = getchar_timeout_us(10000);
        if (c == PICO_ERROR_TIMEOUT)
            continue;

        if (c == '\n' || idx >= (int)sizeof(buf) - 1) {
            buf[idx] = '\0';
            idx = 0;
            if (strncmp(buf, "TIME:", 5) == 0) {
                ds3231_datetime_t dt = {0};
                int dotw;
                int parsed = sscanf(buf + 5, "%hu-%hhu-%hhuT%hhu:%hhu:%hhu:%d", &dt.year,
                                    &dt.month, &dt.day, &dt.hour, &dt.minutes, &dt.seconds,
                                    &dotw);
                if (parsed == 7) {
                    dt.dotw = (uint8_t)dotw;
                    ds3231_set_datetime(&dt, rtc);
                    printf("RTC synced: %04d-%02d-%02dT%02d:%02d:%02d\n", dt.year, dt.month,
                           dt.day, dt.hour, dt.minutes, dt.seconds);
                }
                return;
            }
        } else {
            buf[idx++] = (char)c;
        }
    }

    printf("No time sync received, using existing RTC time\n");
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

    // Initialize DS3231 RTC
    static struct ds3231_rtc rtc;
    ds3231_init(DS3231_I2C_PORT, DS3231_I2C_SDA_PIN, DS3231_I2C_SCL_PIN, &rtc);

    // Sync RTC from PC over UART if available
    sync_rtc_from_uart(&rtc);

    // Initialize MS5803 sensor
    ms5803_inst_t* ms_inst = ms5803_init(ADC_4096);

    // Initialize e-paper display
    init_display();

    // Enter GUI main loop (never returns)
    gui_init(ms_inst, &rtc);
    gui_run();
}
