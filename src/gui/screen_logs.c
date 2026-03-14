#include "gui.h"
#include "GUI_Paint.h"
#include "EPD_1in54_V2.h"
#include "fonts.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include <stdio.h>
#include <string.h>

#define MAX_LOG_FILES 64
#define VISIBLE_ITEMS 7

static char filenames[MAX_LOG_FILES][16];
static FSIZE_t file_sizes[MAX_LOG_FILES];
static int file_count;
static int selected;
static int scroll_offset;
static bool detail_view;  // toggled by SIDE press
static bool sd_mounted;

static void mount_and_scan(void) {
    file_count = 0;
    sd_mounted = false;

    sd_card_t *sd = sd_get_by_num(0);
    FRESULT fr = f_mount(&sd->fatfs, sd->pcName, 1);
    if (fr != FR_OK) {
        printf("SD mount failed: %s\n", FRESULT_str(fr));
        return;
    }
    sd_mounted = true;

    DIR dir;
    FILINFO fno;
    fr = f_opendir(&dir, "/");
    if (fr != FR_OK)
        return;

    while (file_count < MAX_LOG_FILES) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0)
            break;
        if (fno.fattrib & AM_DIR)
            continue;
        // Only show .bin files
        const char *ext = strrchr(fno.fname, '.');
        if (!ext || strcmp(ext, ".bin") != 0)
            continue;

        strncpy(filenames[file_count], fno.fname, sizeof(filenames[0]) - 1);
        filenames[file_count][sizeof(filenames[0]) - 1] = '\0';
        file_sizes[file_count] = fno.fsize;
        file_count++;
    }
    f_closedir(&dir);
}

static void logs_on_enter(void) {
    selected = 0;
    scroll_offset = 0;
    detail_view = false;
    mount_and_scan();
}

static void logs_on_exit(void) {
    if (sd_mounted) {
        sd_card_t *sd = sd_get_by_num(0);
        f_unmount(sd->pcName);
        sd_mounted = false;
    }
}

static void logs_render(void) {
    Paint_Clear(WHITE);

    // Title with count
    char title[32];
    snprintf(title, sizeof(title), "DIVE LOGS (%d)", file_count);
    Paint_DrawString_EN(5, 5, title, &Font16, WHITE, BLACK);

    if (file_count == 0) {
        Paint_DrawString_EN(20, 80, "No logs found", &Font16, WHITE, BLACK);
        Paint_DrawString_EN(5, 185, "HOLD SIDE: menu", &Font12, WHITE, BLACK);
        EPD_1IN54_V2_DisplayPart(Paint.Image);
        return;
    }

    if (detail_view) {
        // Detail view for selected file
        UWORD y = 35;
        Paint_DrawString_EN(5, y, filenames[selected], &Font16, WHITE, BLACK);
        y += Font16.Height + 8;

        char size_str[32];
        snprintf(size_str, sizeof(size_str), "Size: %lu B", (unsigned long)file_sizes[selected]);
        Paint_DrawString_EN(10, y, size_str, &Font12, WHITE, BLACK);
        y += Font12.Height + 4;

        // Calculate entry count (15 bytes per entry)
        unsigned long entries = (unsigned long)file_sizes[selected] / 15;
        char entries_str[32];
        snprintf(entries_str, sizeof(entries_str), "Entries: %lu", entries);
        Paint_DrawString_EN(10, y, entries_str, &Font12, WHITE, BLACK);
        y += Font12.Height + 4;

        // Duration estimate (1 entry per second)
        unsigned long mins = entries / 60;
        unsigned long secs = entries % 60;
        char dur_str[32];
        snprintf(dur_str, sizeof(dur_str), "~%lum %lus", mins, secs);
        Paint_DrawString_EN(10, y, dur_str, &Font12, WHITE, BLACK);

        Paint_DrawString_EN(5, 185, "SIDE:back HOLD:menu", &Font12, WHITE, BLACK);
    } else {
        // List view
        UWORD y = 30;
        UWORD item_h = Font12.Height + 8;

        for (int i = 0; i < VISIBLE_ITEMS && (scroll_offset + i) < file_count; i++) {
            int idx = scroll_offset + i;
            UWORD iy = y + i * item_h;

            if (idx == selected) {
                Paint_DrawRectangle(3, iy - 1, 197, iy + Font12.Height + 1, BLACK, DOT_PIXEL_1X1,
                                    DRAW_FILL_FULL);
                Paint_DrawString_EN(5, iy, filenames[idx], &Font12, BLACK, WHITE);
            } else {
                Paint_DrawString_EN(5, iy, filenames[idx], &Font12, WHITE, BLACK);
            }
        }

        // Scroll indicator
        if (file_count > VISIBLE_ITEMS) {
            char scroll_str[16];
            snprintf(scroll_str, sizeof(scroll_str), "%d/%d", selected + 1, file_count);
            UWORD sx = 200 - strlen(scroll_str) * Font12.Width - 5;
            Paint_DrawString_EN(sx, 170, scroll_str, &Font12, WHITE, BLACK);
        }

        Paint_DrawString_EN(5, 185, "TOP:nav SIDE:view", &Font12, WHITE, BLACK);
    }

    EPD_1IN54_V2_DisplayPart(Paint.Image);
}

static void logs_update(void) {
    // No periodic update needed
}

static void logs_on_top_press(void) {
    if (detail_view) {
        // In detail view, TOP goes back to list
        detail_view = false;
    } else {
        // Scroll through files
        if (file_count > 0) {
            selected = (selected + 1) % file_count;
            // Adjust scroll window
            if (selected < scroll_offset) {
                scroll_offset = selected;
            } else if (selected >= scroll_offset + VISIBLE_ITEMS) {
                scroll_offset = selected - VISIBLE_ITEMS + 1;
            }
            // Handle wrap-around
            if (selected == 0) {
                scroll_offset = 0;
            }
        }
    }
    logs_render();
}

static void logs_on_side_press(void) {
    if (detail_view) {
        // Back to list view
        detail_view = false;
    } else if (file_count > 0) {
        // Show detail for selected file
        detail_view = true;
    }
    logs_render();
}

static void logs_on_side_long(void) {
    gui_set_screen(&screen_menu);
}

screen_t screen_logs = {
    .on_enter = logs_on_enter,
    .on_exit = logs_on_exit,
    .render = logs_render,
    .update = logs_update,
    .on_top_press = logs_on_top_press,
    .on_side_press = logs_on_side_press,
    .on_side_long = logs_on_side_long,
};
