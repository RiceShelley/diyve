#ifndef DIVE_LOGGER__H
#define DIVE_LOGGER__H

#include <stdint.h>

typedef struct dive_log_inst dive_log_inst_t;

typedef struct time {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} time_t;

typedef struct dive_log_entry {
    time_t timestamp;
    float temperature_c;
    float depth_m; 
} dive_log_entry_t;

int dive_logger_start();
int dive_logger_record(dive_log_entry_t* entry);
int dive_logger_stop();

#endif
