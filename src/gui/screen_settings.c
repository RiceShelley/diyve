#include "gui.h"
#include "GUI_Paint.h"
#include "EPD_1in54_V2.h"
#include "fonts.h"
#include "ff.h"
#include "hw_config.h"
#include <stdio.h>
#include <string.h>

#define SETTINGS_FILE "settings.txt"

static void settings_save(void) {
    sd_card_t *sd = sd_get_by_num(0);
    if (f_mount(&sd->fatfs, sd->pcName, 1) != FR_OK)
        return;

    FIL f;
    if (f_open(&f, SETTINGS_FILE, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        char buf[64];
        int len;

        len = snprintf(buf, sizeof(buf), "pressure_offset=%ld\n",
                       (long)pressure_calibration_offset_pa);
        UINT bw;
        f_write(&f, buf, len, &bw);

        len = snprintf(buf, sizeof(buf), "water_density=%d\n",
                       (water_density_kg_m3 > 1010.0f) ? 1025 : 1000);
        f_write(&f, buf, len, &bw);

        len = snprintf(buf, sizeof(buf), "units_imperial=%d\n", units_imperial ? 1 : 0);
        f_write(&f, buf, len, &bw);

        f_close(&f);
    }

    f_unmount(sd->pcName);
}

void settings_load(void) {
    sd_card_t *sd = sd_get_by_num(0);
    if (f_mount(&sd->fatfs, sd->pcName, 1) != FR_OK)
        return;

    FIL f;
    if (f_open(&f, SETTINGS_FILE, FA_READ) == FR_OK) {
        char line[64];
        while (f_gets(line, sizeof(line), &f)) {
            long ival;
            if (sscanf(line, "pressure_offset=%ld", &ival) == 1) {
                if (ival >= -500 && ival <= 500)
                    pressure_calibration_offset_pa = (int32_t)ival;
            } else if (sscanf(line, "water_density=%ld", &ival) == 1) {
                water_density_kg_m3 = (ival == 1000) ? 1000.0f : 1025.0f;
            } else if (sscanf(line, "units_imperial=%ld", &ival) == 1) {
                units_imperial = (ival != 0);
            }
        }
        f_close(&f);
    }

    f_unmount(sd->pcName);
}

#define SETTING_COUNT 3
#define SETTING_PRESSURE 0
#define SETTING_WATER    1
#define SETTING_UNITS    2

#define PRESSURE_MIN -500
#define PRESSURE_MAX  500
#define PRESSURE_STEP  100

static int selected = 0;
static bool editing = false;

static void settings_render(void) {
    Paint_Clear(WHITE);

    // Title
    UWORD title_x = (200 - Font24.Width * 8) / 2;
    Paint_DrawString_EN(title_x, 10, "SETTINGS", &Font24, WHITE, BLACK);

    // Setting rows: y positions
    const UWORD ys[SETTING_COUNT] = {50, 95, 130};
    const char *labels[SETTING_COUNT] = {
        "Pressure offset",
        "Water type",
        "Units",
    };

    // Value strings
    char pressure_val[20];
    snprintf(pressure_val, sizeof(pressure_val), "%+ld Pa",
             (long)pressure_calibration_offset_pa);
    const char *water_val = (water_density_kg_m3 > 1010.0f) ? "Saltwater" : "Freshwater";
    const char *units_val = units_imperial ? "Imperial" : "Metric";
    const char *values[SETTING_COUNT] = {pressure_val, water_val, units_val};

    for (int i = 0; i < SETTING_COUNT; i++) {
        UWORD y = ys[i];
        if (i == selected) {
            // Highlighted label
            Paint_DrawRectangle(3, y - 2, 197, y + Font16.Height + 2, BLACK,
                                DOT_PIXEL_1X1, DRAW_FILL_FULL);
            Paint_DrawString_EN(5, y, labels[i], &Font16, BLACK, WHITE);
            // Value below label
            UWORD vy = y + Font16.Height + 2;
            if (i == SETTING_PRESSURE && editing) {
                // Show value with edit indicator
                char edit_str[28];
                snprintf(edit_str, sizeof(edit_str), "[ %s ]", pressure_val);
                Paint_DrawString_EN(10, vy, edit_str, &Font12, WHITE, BLACK);
            } else {
                Paint_DrawString_EN(10, vy, values[i], &Font12, WHITE, BLACK);
            }
        } else {
            Paint_DrawString_EN(5, y, labels[i], &Font16, WHITE, BLACK);
        }
    }

    // Controls hint
    if (editing) {
        Paint_DrawString_EN(5, 185, "TOP:confirm SIDE:+10", &Font12, WHITE, BLACK);
    } else {
        Paint_DrawString_EN(5, 185, "TOP:nav  SIDE:edit", &Font12, WHITE, BLACK);
    }

    EPD_1IN54_V2_DisplayPart(Paint.Image);
}

static void settings_on_enter(void) {
    selected = 0;
    editing = false;
}

static void settings_on_exit(void) {
    settings_save();
}

static void settings_on_top_press(void) {
    if (editing) {
        // Confirm edit, return to browse
        editing = false;
    } else {
        selected = (selected + 1) % SETTING_COUNT;
    }
    settings_render();
}

static void settings_on_side_press(void) {
    if (editing) {
        // Increment pressure offset by step, clamp
        pressure_calibration_offset_pa += PRESSURE_STEP;
        if (pressure_calibration_offset_pa > PRESSURE_MAX)
            pressure_calibration_offset_pa = PRESSURE_MAX;
    } else if (selected == SETTING_PRESSURE) {
        editing = true;
    } else if (selected == SETTING_WATER) {
        // Toggle between saltwater and freshwater
        water_density_kg_m3 =
            (water_density_kg_m3 > 1010.0f) ? 1000.0f : 1025.0f;
    } else if (selected == SETTING_UNITS) {
        units_imperial = !units_imperial;
    }
    settings_render();
}

static void settings_on_side_long(void) {
    if (editing) {
        // Decrement pressure offset by step, clamp
        pressure_calibration_offset_pa -= PRESSURE_STEP;
        if (pressure_calibration_offset_pa < PRESSURE_MIN)
            pressure_calibration_offset_pa = PRESSURE_MIN;
        settings_render();
    } else {
        gui_set_screen(&screen_menu);
    }
}

screen_t screen_settings = {
    .on_enter = settings_on_enter,
    .on_exit = settings_on_exit,
    .render = settings_render,
    .update = NULL,
    .on_top_press = settings_on_top_press,
    .on_side_press = settings_on_side_press,
    .on_side_long = settings_on_side_long,
};
