#ifndef GUI__H
#define GUI__H

#include "ms5803_i2c.h"
#include "ds3231.h"

// Screen interface — each screen implements these callbacks
typedef struct screen {
    void (*on_enter)(void);
    void (*on_exit)(void);
    void (*render)(void);
    void (*update)(void);
    void (*on_top_press)(void);
    void (*on_side_press)(void);
    void (*on_side_long)(void);
} screen_t;

// Shared hardware state accessible by all screens
typedef struct {
    ms5803_inst_t *ms;
    struct ds3231_rtc *rtc;
} gui_hw_t;

extern gui_hw_t gui_hw;

// Screen instances (defined in screen_*.c files)
extern screen_t screen_menu;
extern screen_t screen_dive;
extern screen_t screen_logs;
extern screen_t screen_settings;

// Transition to a new screen (calls on_exit on old, on_enter on new)
void gui_set_screen(screen_t *screen);

// Initialize GUI state and enter the main menu
void gui_init(ms5803_inst_t *ms, struct ds3231_rtc *rtc);

// Main loop — never returns
void gui_run(void);

// Adjustable globals (modified by screen_settings)
extern int32_t pressure_calibration_offset_pa;
extern float water_density_kg_m3;
extern bool units_imperial;

// Load settings from SD card (settings.txt); safe to call if SD absent
void settings_load(void);

// Shared utilities
float calc_depth_in_m(uint32_t pa, float water_density_kg_m3);

#endif
