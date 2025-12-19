#ifndef DIVE_DISPLAY__H
#define DIVE_DISPLAY__H

#include "dive_logger.h"

int init_display();
void display_dive_entry(dive_log_entry_t* entry);

#endif
