#include "gui.h"
#include "dive_display.h"
#include "dive_logger.h"
#include "GUI_Paint.h"
#include "EPD_1in54_V2.h"
#include "fonts.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include <stdio.h>
#include <string.h>

// Draw a single character from a font scaled by 2x
static void draw_char_2x(UWORD x, UWORD y, char c, sFONT *font, UWORD fg, UWORD bg) {
    uint16_t bytes_per_row = (font->Width + 7) / 8;
    uint32_t offset = (c - ' ') * font->Height * bytes_per_row;
    const uint8_t *data = &font->table[offset];

    for (uint16_t row = 0; row < font->Height; row++) {
        for (uint16_t col = 0; col < font->Width; col++) {
            uint8_t byte = data[row * bytes_per_row + col / 8];
            bool set = byte & (0x80 >> (col % 8));
            UWORD color = set ? fg : bg;
            // Draw 2x2 block
            Paint_SetPixel(x + col * 2, y + row * 2, color);
            Paint_SetPixel(x + col * 2 + 1, y + row * 2, color);
            Paint_SetPixel(x + col * 2, y + row * 2 + 1, color);
            Paint_SetPixel(x + col * 2 + 1, y + row * 2 + 1, color);
        }
    }
}

// Draw a string scaled by 2x
static void draw_string_2x(UWORD x, UWORD y, const char *str, sFONT *font, UWORD fg, UWORD bg) {
    while (*str) {
        draw_char_2x(x, y, *str, font, fg, bg);
        x += font->Width * 2;
        str++;
    }
}

static dive_log_entry_t log_entry;
static uint32_t dive_start_sec;  // elapsed-time origin (from RTC)
static float max_depth_m;
static bool logger_active;

// Find next available dive_NNN.bin filename
static void make_dive_filename(char *buf, size_t len) {
    sd_card_t *sd = sd_get_by_num(0);
    FRESULT fr = f_mount(&sd->fatfs, sd->pcName, 1);
    if (fr != FR_OK) {
        snprintf(buf, len, "dive_000.bin");
        return;
    }

    DIR dir;
    FILINFO fno;
    int max_num = -1;

    fr = f_opendir(&dir, "/");
    if (fr == FR_OK) {
        while (true) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0)
                break;
            int n;
            if (sscanf(fno.fname, "dive_%d.bin", &n) == 1) {
                if (n > max_num)
                    max_num = n;
            }
        }
        f_closedir(&dir);
    }

    f_unmount(sd->pcName);
    snprintf(buf, len, "dive_%03d.bin", max_num + 1);
}

static uint32_t rtc_to_seconds(ds3231_datetime_t *dt) {
    return (uint32_t)dt->hour * 3600 + (uint32_t)dt->minutes * 60 + dt->seconds;
}

static void dive_on_enter(void) {
    max_depth_m = 0.0f;
    logger_active = false;

    memset(&log_entry, 0, sizeof(log_entry));

    // Get start time
    ds3231_datetime_t dt;
    ds3231_get_datetime(&dt, gui_hw.rtc);
    dive_start_sec = rtc_to_seconds(&dt);

    // Start logger with auto-generated filename
    char filename[32];
    make_dive_filename(filename, sizeof(filename));
    printf("Starting dive: %s\n", filename);
    if (dive_logger_start(filename) == 0) {
        logger_active = true;
    }
}

static void dive_on_exit(void) {
    if (logger_active) {
        dive_logger_stop();
        logger_active = false;
    }
}

static void dive_render(void) {
    Paint_Clear(WHITE);

    // Elapsed time + temperature at top
    ds3231_datetime_t dt;
    ds3231_get_datetime(&dt, gui_hw.rtc);
    uint32_t now_sec = rtc_to_seconds(&dt);
    uint32_t elapsed = now_sec - dive_start_sec;
    uint32_t e_min = elapsed / 60;
    uint32_t e_sec = elapsed % 60;

    char top_str[32];
    if (units_imperial) {
        float temp_f = c_to_f(log_entry.temperature_c);
        snprintf(top_str, sizeof(top_str), "%02lu:%02lu   %dF", (unsigned long)e_min,
                 (unsigned long)e_sec, (int)temp_f);
    } else {
        snprintf(top_str, sizeof(top_str), "%02lu:%02lu   %dC", (unsigned long)e_min,
                 (unsigned long)e_sec, (int)log_entry.temperature_c);
    }
    Paint_DrawString_EN(5, 5, top_str, &Font16, WHITE, BLACK);

    // Large depth number centered at 2x Font24 (34x48 per char)
    float depth_display = units_imperial ? m_to_ft(log_entry.depth_m) : log_entry.depth_m;
    const char *depth_unit = units_imperial ? "ft" : "m";
    if (depth_display < 0)
        depth_display = 0;
    char depth_num[8];
    snprintf(depth_num, sizeof(depth_num), "%d", (int)depth_display);
    UWORD scaled_char_w = Font24.Width * 2;
    UWORD scaled_h = Font24.Height * 2;
    UWORD num_w = strlen(depth_num) * scaled_char_w;
    UWORD unit_w = strlen(depth_unit) * Font16.Width;
    UWORD total_w = num_w + 4 + unit_w;
    UWORD depth_x = (200 - total_w) / 2;
    UWORD depth_y = 55;
    draw_string_2x(depth_x, depth_y, depth_num, &Font24, BLACK, WHITE);
    Paint_DrawString_EN(depth_x + num_w + 4, depth_y + scaled_h - Font16.Height, depth_unit,
                        &Font16, WHITE, BLACK);

    // Max depth + wall clock at bottom
    float max_display = units_imperial ? m_to_ft(max_depth_m) : max_depth_m;
    char bottom_str[40];
    if (units_imperial) {
        snprintf(bottom_str, sizeof(bottom_str), "Max:%dft  %02d:%02d:%02d", (int)max_display,
                 dt.hour, dt.minutes, dt.seconds);
    } else {
        snprintf(bottom_str, sizeof(bottom_str), "Max:%dm  %02d:%02d:%02d", (int)max_display,
                 dt.hour, dt.minutes, dt.seconds);
    }
    Paint_DrawString_EN(5, 165, bottom_str, &Font12, WHITE, BLACK);

    // Controls hint
    Paint_DrawString_EN(5, 185, "HOLD SIDE: end dive", &Font12, WHITE, BLACK);

    // Warn if SD card is not present and dive is not being logged
    if (!logger_active) {
        Paint_DrawString_EN(5, 145, "NO SD", &Font12, BLACK, WHITE);
    }

    EPD_1IN54_V2_DisplayPart(Paint.Image);
}

static void dive_update(void) {
    // Read sensors
    ms5803_update(gui_hw.ms);
    log_entry.temperature_c = ms5803_get_temp(gui_hw.ms, CELSIUS);
    uint32_t pressure = ms5803_get_pressure(gui_hw.ms);
    log_entry.depth_m = calc_depth_in_m(pressure, water_density_kg_m3);

    if (log_entry.depth_m > max_depth_m) {
        max_depth_m = log_entry.depth_m;
    }

    // Update timestamp
    ds3231_datetime_t dt;
    ds3231_get_datetime(&dt, gui_hw.rtc);
    log_entry.timestamp.year = dt.year;
    log_entry.timestamp.month = dt.month;
    log_entry.timestamp.day = dt.day;
    log_entry.timestamp.hour = dt.hour;
    log_entry.timestamp.min = dt.minutes;
    log_entry.timestamp.sec = dt.seconds;

    // Log to SD
    if (logger_active) {
        dive_logger_record(&log_entry);
    }

    dive_render();
}

static void dive_on_top_press(void) {
    // No action during dive
}

static void dive_on_side_press(void) {
    // No action on short press during dive
}

static void dive_on_side_long(void) {
    gui_set_screen(&screen_menu);
}

screen_t screen_dive = {
    .on_enter = dive_on_enter,
    .on_exit = dive_on_exit,
    .render = dive_render,
    .update = dive_update,
    .on_top_press = dive_on_top_press,
    .on_side_press = dive_on_side_press,
    .on_side_long = dive_on_side_long,
};
