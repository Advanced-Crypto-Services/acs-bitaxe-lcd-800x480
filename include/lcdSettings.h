#pragma once
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>
#include "driver/ledc.h"


extern bool backlightPWM;

extern void initializeIOExpander(void);
extern void setBrightness(uint8_t value);
extern uint8_t getBrightness(void);
