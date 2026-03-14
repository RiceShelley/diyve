#include "dive_display.h"

#include "EPD_1in54_V2.h"
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"

static UBYTE BlackImage[(EPD_1IN54_V2_WIDTH / 8) * EPD_1IN54_V2_HEIGHT];
static UWORD Imagesize = (EPD_1IN54_V2_WIDTH / 8) * EPD_1IN54_V2_HEIGHT;

int init_display() {
    printf("EPD_1in54_V2_test Demo\r\n");
    if (DEV_Module_Init() != 0) {
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_1IN54_V2_Init();

    EPD_1IN54_V2_Clear();
    DEV_Delay_ms(100);

    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_1IN54_V2_WIDTH, EPD_1IN54_V2_HEIGHT, 270, WHITE);
    Paint_Clear(WHITE);
    return 0;
}

float c_to_f(float c) {
    return (c * 9.0f / 5.0f) + 32.0f;
}

float m_to_ft(float m) {
    return m * 3.28084f;
}

static void display_labeled_int(UWORD x, UWORD y, const char* label, size_t label_len, int32_t num,
                                sFONT* font) {
    Paint_ClearWindows(x, y, x + font->Width * (label_len + 4), y + font->Height, WHITE);
    Paint_DrawString_EN(x, y, label, &Font20, WHITE, BLACK);
    Paint_DrawNum(x + label_len * Font20.Width, y, num, font, BLACK, WHITE);
}

void display_dive_entry(dive_log_entry_t* entry) {
    sFONT* font = &Font20;

    UWORD y_margin = 10;

    UWORD time_x = 50;
    UWORD time_y = 5;

    UWORD temp_x = 5;
    UWORD temp_y = time_y + font->Height + y_margin;
    const char temp_disp_str[] = "Temp F:";

    UWORD depth_x = 5;
    UWORD depth_y = temp_y + font->Height + y_margin;
    const char depth_disp_str[] = "Depth ft:";

    EPD_1IN54_V2_DisplayPartBaseImage(BlackImage);

    // enter partial mode
    EPD_1IN54_V2_Init_Partial();

    PAINT_TIME display_time;

    display_time.Year = entry->timestamp.year;
    display_time.Month = entry->timestamp.month;
    display_time.Day = entry->timestamp.day;
    display_time.Hour = entry->timestamp.hour;
    display_time.Min = entry->timestamp.min;
    display_time.Sec = entry->timestamp.sec;

    Paint_ClearWindows(time_x, time_y, time_x + font->Width * 7, time_y + font->Height, WHITE);
    Paint_DrawTime(time_x, time_y, &display_time, font, WHITE, BLACK);

    // Draw Temperature
    display_labeled_int(temp_x, temp_y, temp_disp_str, sizeof(temp_disp_str),
                        (int)c_to_f(entry->temperature_c), font);

    // Draw Depth
    display_labeled_int(depth_x, depth_y, depth_disp_str, sizeof(depth_disp_str),
                        (int)m_to_ft(entry->depth_m), font);

    EPD_1IN54_V2_Display(BlackImage);
}
