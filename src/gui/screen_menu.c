#include "gui.h"
#include "dive_display.h"
#include "GUI_Paint.h"
#include "EPD_1in54_V2.h"
#include "fonts.h"
#include "ff.h"
#include "hw_config.h"
#include <stdio.h>
#include <string.h>

#define MENU_ITEM_COUNT 3

static const char *menu_items[MENU_ITEM_COUNT] = {
    "New Dive",
    "Dive Logs",
    "Settings",
};

static int selected = 0;
static bool sd_present = false;

static bool check_sd(void) {
    sd_card_t *sd = sd_get_by_num(0);
    FRESULT fr = f_mount(&sd->fatfs, sd->pcName, 1);
    if (fr != FR_OK) {
        return false;
    }
    f_unmount(sd->pcName);
    return true;
}

static void menu_draw(void) {
    Paint_Clear(WHITE);

    // Title "DIYVE" centered (Font24 width=17, 5 chars = 85px, center in 200px)
    UWORD title_x = (200 - Font24.Width * 5) / 2;
    Paint_DrawString_EN(title_x, 10, "DIYVE", &Font24, WHITE, BLACK);

    // Menu items
    UWORD item_y = 50;
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        UWORD y = item_y + i * (Font20.Height + 12);
        if (i == selected) {
            // Inverted: black rect with white text
            Paint_DrawRectangle(5, y - 2, 195, y + Font20.Height + 2, BLACK, DOT_PIXEL_1X1,
                                DRAW_FILL_FULL);
            Paint_DrawString_EN(10, y, menu_items[i], &Font20, BLACK, WHITE);
        } else {
            Paint_DrawString_EN(10, y, menu_items[i], &Font20, WHITE, BLACK);
        }
    }

    // Status bar at bottom: time + pressure
    ds3231_datetime_t dt;
    ds3231_get_datetime(&dt, gui_hw.rtc);

    char time_str[20];
    snprintf(time_str, sizeof(time_str), "%02d:%02d %02d/%02d/%04d", dt.hour, dt.minutes, dt.month,
             dt.day, dt.year);
    Paint_DrawString_EN(5, 160, time_str, &Font12, WHITE, BLACK);

    ms5803_update(gui_hw.ms);
    uint32_t pressure_pa = ms5803_get_pressure(gui_hw.ms);
    float psi = (float)pressure_pa / 6894.76f;
    char psi_str[16];
    snprintf(psi_str, sizeof(psi_str), "%.1f psi", psi);
    UWORD psi_x = 200 - (strlen(psi_str) * Font12.Width) - 5;
    Paint_DrawString_EN(psi_x, 160, psi_str, &Font12, WHITE, BLACK);

    // Controls hint
    Paint_DrawString_EN(5, 185, "TOP:nav  SIDE:select", &Font12, WHITE, BLACK);

    // Warn if no SD card detected
    if (!sd_present) {
        Paint_DrawString_EN(5, 145, "NO SD CARD", &Font12, BLACK, WHITE);
    }
}

static void menu_on_enter(void) {
    selected = 0;
    sd_present = check_sd();
}

static void menu_on_exit(void) {
}

static void menu_render(void) {
    menu_draw();
    EPD_1IN54_V2_DisplayPart(Paint.Image);
}

static void menu_update(void) {
    // No periodic update needed on menu
}

static void menu_on_top_press(void) {
    selected = (selected + 1) % MENU_ITEM_COUNT;
    menu_render();
}

static void menu_on_side_press(void) {
    if (selected == 0) {
        gui_set_screen(&screen_dive);
    } else if (selected == 1) {
        gui_set_screen(&screen_logs);
    } else if (selected == 2) {
        gui_set_screen(&screen_settings);
    }
}

static void menu_on_side_long(void) {
    // No action on long press from menu
}

screen_t screen_menu = {
    .on_enter = menu_on_enter,
    .on_exit = menu_on_exit,
    .render = menu_render,
    .update = menu_update,
    .on_top_press = menu_on_top_press,
    .on_side_press = menu_on_side_press,
    .on_side_long = menu_on_side_long,
};
