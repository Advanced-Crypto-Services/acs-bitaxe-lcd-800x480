#pragma once
#include <lvgl.h>
#include "wifiFeatures.h"

extern bool enableClockScreen;
extern bool enableBlockScreen;
extern bool enableBtcPriceScreen;

extern void clockContainerEventCallback(lv_event_t* e);

extern void homeScreen();
