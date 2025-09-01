#pragma once
#include <lvgl.h>
#include "wifiFeatures.h"

#define MINING_GRAPH_MAX_VARIATIONS 3  // Number of graph variations

extern void miningStatsScreen();

struct MiningGraphContainerState {
    lv_obj_t* containers[MINING_GRAPH_MAX_VARIATIONS];
    lv_timer_t* timers[MINING_GRAPH_MAX_VARIATIONS];
    int currentIndex;
};

extern MiningGraphContainerState miningGraphState;
