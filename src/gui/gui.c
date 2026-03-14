#include "gui.h"
#include "pico/stdlib.h"
#include "diyve_pins.h"
#include "GUI_Paint.h"
#include "EPD_1in54_V2.h"
#include <stdio.h>

// Poll interval
#define POLL_MS 50

// Debounce: require held for 3 consecutive polls (~150ms)
#define DEBOUNCE_COUNT 3

// Long press: held for 20 polls (~1 second)
#define LONG_PRESS_COUNT 20

// Update tick: call screen->update every N polls (~1 second)
#define UPDATE_INTERVAL 20

gui_hw_t gui_hw;

static screen_t *current_screen;

// Button state
static uint8_t top_held_count;
static uint8_t side_held_count;
static bool top_fired;
static bool side_fired;
static bool side_long_fired;

// Sensor calibration offset in Pa (-500 to +500, step 10).
int32_t pressure_calibration_offset_pa = 200;

// Water density: 1025.0 = saltwater, 1000.0 = freshwater.
float water_density_kg_m3 = 1025.0f;

// Display units: true = imperial (ft/°F), false = metric (m/°C).
bool units_imperial = true;

float calc_depth_in_m(uint32_t pa, float water_density_kg_m3) {
    uint32_t atmospheric_pressure_pa = 101325 + pressure_calibration_offset_pa;
    int32_t gauge_pressure_pa = (int32_t)pa - (int32_t)atmospheric_pressure_pa;
    return ((float)gauge_pressure_pa) / (water_density_kg_m3 * 9.81f);
}

void gui_set_screen(screen_t *screen) {
    if (current_screen && current_screen->on_exit) {
        current_screen->on_exit();
    }
    current_screen = screen;
    if (current_screen && current_screen->on_enter) {
        current_screen->on_enter();
    }
    if (current_screen && current_screen->render) {
        current_screen->render();
    }
}

void gui_init(ms5803_inst_t *ms, struct ds3231_rtc *rtc) {
    gui_hw.ms = ms;
    gui_hw.rtc = rtc;

    top_held_count = 0;
    side_held_count = 0;
    top_fired = false;
    side_fired = false;
    side_long_fired = false;

    // Enter partial refresh mode once — all screens use DisplayPart from here on
    EPD_1IN54_V2_DisplayPartBaseImage(Paint.Image);
    EPD_1IN54_V2_Init_Partial();

    settings_load();

    current_screen = NULL;
    gui_set_screen(&screen_menu);
}

static void poll_buttons(void) {
    // TOP button (active low with pull-up)
    if (gpio_get(TOP_BUTTON_PIN) == 0) {
        if (top_held_count < 255)
            top_held_count++;
    } else {
        if (top_fired) {
            // Button released after firing — reset
            top_fired = false;
        }
        top_held_count = 0;
    }

    // SIDE button (active low with pull-up)
    if (gpio_get(SIDE_BUTTON_PIN) == 0) {
        if (side_held_count < 255)
            side_held_count++;
    } else {
        // On release: if we held long enough for short press but didn't fire long
        if (side_held_count >= DEBOUNCE_COUNT && !side_long_fired && !side_fired) {
            side_fired = true;
        }
        side_held_count = 0;
        side_long_fired = false;
    }

    // Fire TOP short press on threshold
    if (top_held_count == DEBOUNCE_COUNT && !top_fired) {
        top_fired = true;
        if (current_screen && current_screen->on_top_press) {
            current_screen->on_top_press();
        }
    }

    // Fire SIDE long press on threshold
    if (side_held_count == LONG_PRESS_COUNT && !side_long_fired) {
        side_long_fired = true;
        if (current_screen && current_screen->on_side_long) {
            current_screen->on_side_long();
        }
    }

    // Fire SIDE short press (on release, handled above)
    if (side_fired) {
        side_fired = false;
        if (current_screen && current_screen->on_side_press) {
            current_screen->on_side_press();
        }
    }
}

void gui_run(void) {
    uint32_t update_tick = 0;

    while (true) {
        poll_buttons();

        update_tick++;
        if (update_tick >= UPDATE_INTERVAL) {
            update_tick = 0;
            if (current_screen && current_screen->update) {
                current_screen->update();
            }
        }

        sleep_ms(POLL_MS);
    }
}
