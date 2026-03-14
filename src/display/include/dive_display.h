#ifndef DIVE_DISPLAY__H
#define DIVE_DISPLAY__H

#include "dive_logger.h"

int init_display();
void display_dive_entry(dive_log_entry_t* entry);

// Unit conversion utilities
float c_to_f(float c);
float m_to_ft(float m);

#endif
