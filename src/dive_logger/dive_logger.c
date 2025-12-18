#include "include/dive_logger.h"
#include "f_util.h"
#include "ff.h"
#include "pico/stdlib.h"
#include "rtc.h"
#include "hw_config.h"
#include <string.h>
#include <stdint.h>

struct dive_log_inst {
    const char* log_filename;
    sd_card_t *pSD;
    FIL fil;
};

static uint8_t dive_log_entry_buff[sizeof(dive_log_entry_t)];

static dive_log_inst_t dl_state;

int dive_logger_start(const char* log_filename) {

    dl_state.log_filename = log_filename;
    dl_state.pSD = sd_get_by_num(0);

    // Mount SD Card
    FRESULT fr = f_mount(&dl_state.pSD->fatfs, dl_state.pSD->pcName, 1);
    if (FR_OK != fr) {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return 1;
    }

    // Open log file
    fr = f_open(&dl_state.fil, dl_state.log_filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", dl_state.log_filename, FRESULT_str(fr), fr);
        return 1;
    }

    return 0;
}

size_t pack_dive_log_entry(dive_log_entry_t* entry, uint8_t* buffer) {
    size_t offset = 0;
    size_t elem_size;

    // Year
    elem_size = sizeof(entry->timestamp.year);
    memcpy(buffer + offset, &entry->timestamp.year, elem_size);
    offset += elem_size;

    // Month
    elem_size = sizeof(entry->timestamp.month);
    memcpy(buffer + offset, &entry->timestamp.month, elem_size);
    offset += elem_size;

    // Day
    elem_size = sizeof(entry->timestamp.day);
    memcpy(buffer + offset, &entry->timestamp.day, elem_size);
    offset += elem_size;

    // Hour
    elem_size = sizeof(entry->timestamp.hour);
    memcpy(buffer + offset, &entry->timestamp.hour, elem_size);
    offset += elem_size;

    // Min
    elem_size = sizeof(entry->timestamp.min);
    memcpy(buffer + offset, &entry->timestamp.min, elem_size);
    offset += elem_size;

    // Sec
    elem_size = sizeof(entry->timestamp.sec);
    memcpy(buffer + offset, &entry->timestamp.sec, elem_size);
    offset += elem_size;

    // Pack temperature
    elem_size = sizeof(entry->temperature_c);
    memcpy(buffer + offset, &entry->temperature_c, elem_size);
    offset += elem_size;

    // Pack depth
    elem_size = sizeof(entry->depth_m);
    memcpy(buffer + offset, &entry->depth_m, elem_size);
    offset += elem_size;

    return offset;
}

int dive_logger_record(dive_log_entry_t* entry) {
    UINT bw;
    size_t entry_size;

    entry_size = pack_dive_log_entry(entry, dive_log_entry_buff);

    if (f_write(&dl_state.fil, dive_log_entry_buff, entry_size, &bw) < 0) {
        return 1;
    }

    if ((entry_size - bw) != 0) {
        return 1;
    }

    return 0;
}

int dive_logger_stop() {
    FRESULT fr = f_close(&dl_state.fil);

    if (FR_OK != fr) {
        panic("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
        return 1;
    }

    f_unmount(dl_state.pSD->pcName);
    return 0;
}