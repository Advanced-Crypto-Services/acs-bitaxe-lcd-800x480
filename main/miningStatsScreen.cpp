#include <Arduino.h>
#include "homeScreen.h"
#include "UIScreens.h"
#include "UIThemes.h"
#include <TimeLib.h>
#include "Clock.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include "mempoolAPI.h"
#include "BAP.h"
#include "miningStatsScreen.h"

MiningGraphContainerState miningGraphState;

// Global label references for updates
static lv_obj_t* hashrateValueLabel = nullptr;
static lv_obj_t* hashrateBottomLeftLabel = nullptr;
static lv_obj_t* hashrateBottomRightLabel = nullptr;
static lv_obj_t* powerValueLabel = nullptr;
static lv_obj_t* powerBottomLeftLabel = nullptr;
static lv_obj_t* powerBottomRightLabel = nullptr;
static lv_obj_t* tempValueLabel = nullptr;
static lv_obj_t* tempBottomLeftLabel = nullptr;
static lv_obj_t* tempBottomRightLabel = nullptr;

// Hashrate variables for auto-ranging
static float minHashrateSeen = 250;
static float maxHashrateSeen = 400;

// Power variables for auto-ranging
static float minPowerSeen = 10;
static float maxPowerSeen = 20;

// Temperature variables for auto-ranging
static float minTempSeen = 40;
static float maxTempSeen = 65;

static void updateMiningGraphVisibility(int graphIndex) {
    if (lvgl_port_lock(10)) {
        // Hide all graphs
        for (int i = 0; i < MINING_GRAPH_MAX_VARIATIONS; i++) {
            if (miningGraphState.containers[i] != nullptr) {
                lv_obj_add_flag(miningGraphState.containers[i], LV_OBJ_FLAG_HIDDEN);
                // Pause timers for hidden graphs
                if (miningGraphState.timers[i] != nullptr) {
                    lv_timer_pause(miningGraphState.timers[i]);
                }
            }
        }
        
        // Show current graph
        if (miningGraphState.containers[graphIndex] != nullptr) {
            lv_obj_clear_flag(miningGraphState.containers[graphIndex], LV_OBJ_FLAG_HIDDEN);
            // Resume timer for active graph
            if (miningGraphState.timers[graphIndex] != nullptr) {
                lv_timer_resume(miningGraphState.timers[graphIndex]);
            }
        }
        lvgl_port_unlock();
    }
}

static void miningGraphContainerEventCallback(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE)) {
        for (int i = 0; i < MINING_GRAPH_MAX_VARIATIONS; i++) {
            if (obj == miningGraphState.containers[i]) {
                // Advance to next graph
                miningGraphState.currentIndex = (miningGraphState.currentIndex + 1) % MINING_GRAPH_MAX_VARIATIONS;
                updateMiningGraphVisibility(miningGraphState.currentIndex);
                Serial.printf("Switched to mining graph %d\n", miningGraphState.currentIndex);
                return;
            }
        }
    }
}

static void setupHashrateGraph(lv_obj_t* container)
{
    // Create content using framework
    graphContainerContent(
        container,
        "Hashrate",
        "0.00 GH/s",
        "Pool: Connecting...",
        "Reject Rate: 0.00%"
    );

    // Store label references for updates (children are created in order)
    hashrateValueLabel = lv_obj_get_child(container, 1);  // Value label
    hashrateBottomLeftLabel = lv_obj_get_child(container, 3);  // Bottom left
    hashrateBottomRightLabel = lv_obj_get_child(container, 4);  // Bottom right

    // Get chart for initialization
    lv_obj_t* chart = lv_obj_get_child(container, 2);  // Chart
    lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);

    // Initialize with current hashrate data
    for(int i = 0; i < 50; i++) {
        lv_chart_set_next_value(chart, series, IncomingData.mining.hashrate);
    }

     lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    minHashrateSeen,
                    maxHashrateSeen + 250);

    // Create update timer
    miningGraphState.timers[0] = lv_timer_create([](lv_timer_t* timer) {
        // Get values outside the lock
        float currentHashrate = IncomingData.mining.hashrate;
        char poolUrl[128];
        uint16_t poolPort = IncomingData.network.poolPort;
        strlcpy(poolUrl, IncomingData.network.poolUrl, sizeof(poolUrl));
        
        float smoothedHashrate = calculateMovingAverage(currentHashrate);
        float rejectRatePercent = IncomingData.mining.rejectRatePercent;
        
        if (lvgl_port_lock(10)) {
            lv_obj_t* chart = (lv_obj_t*)timer->user_data;
            lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);
            
            // Update labels
            lv_label_set_text_fmt(hashrateValueLabel, "%d.%02d GH/s", (int)currentHashrate, (int)(currentHashrate * 100) % 100);
            lv_label_set_text_fmt(hashrateBottomLeftLabel, "Pool: %s:%d", poolUrl, poolPort);
            lv_label_set_text_fmt(hashrateBottomRightLabel, "Reject Rate: %d.%02d%%", (int)rejectRatePercent, (int)(rejectRatePercent * 100) % 100);
            
            // Auto-range the chart
            if (smoothedHashrate > maxHashrateSeen) {
                
                if (smoothedHashrate > maxHashrateSeen) maxHashrateSeen = smoothedHashrate;
                
                lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    minHashrateSeen,
                    maxHashrateSeen + 250);
            }
            
            lv_chart_set_next_value(chart, series, smoothedHashrate);
            lvgl_port_unlock();
        }
    }, 2000, chart);
}

static void setupPowerGraph(lv_obj_t* container)
{
    // Create content using framework
    graphContainerContent(
        container,
        "Power Consumption",
        "0.00 W",
        "Voltage: 0.000 V",
        "Efficiency: 0 W/TH"
    );

    // Store label references for updates
    powerValueLabel = lv_obj_get_child(container, 1);
    powerBottomLeftLabel = lv_obj_get_child(container, 3);
    powerBottomRightLabel = lv_obj_get_child(container, 4);

    // Get chart for initialization
    lv_obj_t* chart = lv_obj_get_child(container, 2);
    lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    minPowerSeen,
                    maxPowerSeen + 5);

    // Initialize with current power data
    for(int i = 0; i < 50; i++) {
        lv_chart_set_next_value(chart, series, IncomingData.monitoring.powerStats.power);
    }

    // Create update timer
    miningGraphState.timers[1] = lv_timer_create([](lv_timer_t* timer) {
        float currentPower = IncomingData.monitoring.powerStats.power;
        float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage;
        float efficiency = IncomingData.mining.efficiency;
        
        if (lvgl_port_lock(10)) {
            lv_obj_t* chart = (lv_obj_t*)timer->user_data;
            lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);
            
            // Update labels
            lv_label_set_text_fmt(powerValueLabel, "%d.%02d W", (int)currentPower, (int)(currentPower * 100) % 100);
            lv_label_set_text_fmt(powerBottomLeftLabel, "Voltage: %d.%03d V", (int)(domainVoltage/1000), (int)(domainVoltage) % 1000);
            lv_label_set_text_fmt(powerBottomRightLabel, "Efficiency: %d W/TH", (int)efficiency);
            
            // Auto-range the chart
            if (currentPower > maxPowerSeen) {
                if (currentPower > maxPowerSeen) maxPowerSeen = currentPower;
                
                lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    minPowerSeen,
                    maxPowerSeen + 5);
            }
            
            lv_chart_set_next_value(chart, series, currentPower);
            lvgl_port_unlock();
        }
    }, 2000, chart);
}

static void setupTemperatureGraph(lv_obj_t* container)
{
    // Create content using framework
    graphContainerContent(
        container,
        "Temperature",
        "0C",
        "Fan: 0 RPM (0%)",
        "VREG: 0C"
    );

    // Store label references for updates
    tempValueLabel = lv_obj_get_child(container, 1);
    tempBottomLeftLabel = lv_obj_get_child(container, 3);
    tempBottomRightLabel = lv_obj_get_child(container, 4);

    // Get chart for initialization
    lv_obj_t* chart = lv_obj_get_child(container, 2);
    lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);

    // Initialize with current temperature data
    for(int i = 0; i < 50; i++) {
        lv_chart_set_next_value(chart, series, IncomingData.monitoring.temperatures[0]);
    }

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    (minTempSeen > 10) ? minTempSeen - 10 : 0,
                    maxTempSeen + 5);

    // Create update timer
    miningGraphState.timers[2] = lv_timer_create([](lv_timer_t* timer) {
        float asicTemp = IncomingData.monitoring.temperatures[0];
        float fanSpeed = IncomingData.monitoring.fanSpeed;
        float fanPercent = IncomingData.monitoring.fanSpeedPercent;
        float vregTemp = IncomingData.monitoring.vregTemp;
        
        if (lvgl_port_lock(10)) {
            lv_obj_t* chart = (lv_obj_t*)timer->user_data;
            lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);
            
            // Update labels
            lv_label_set_text_fmt(tempValueLabel, "%d C", (int)asicTemp);
            lv_label_set_text_fmt(tempBottomLeftLabel, "Fan: %d RPM (%d%%)", (int)fanSpeed, (int)fanPercent);
            lv_label_set_text_fmt(tempBottomRightLabel, "VREG: %dC", (int)vregTemp);
            
            // Auto-range the chart
            if (asicTemp > maxTempSeen) {
                if (asicTemp > maxTempSeen) maxTempSeen = asicTemp;
                
                lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    minTempSeen,
                    maxTempSeen + 5);
            }
            
            lv_chart_set_next_value(chart, series, asicTemp);
            lvgl_port_unlock();
        }
    }, 2000, chart);
}

void miningStatsScreen()
{
    Serial.println("Setting up mining stats screen with graph framework");
    
    // Initialize graph state
    miningGraphState.currentIndex = 0;
    for (int i = 0; i < MINING_GRAPH_MAX_VARIATIONS; i++) {
        miningGraphState.containers[i] = nullptr;
        miningGraphState.timers[i] = nullptr;
    }

    // Create graph containers using framework
    graphContainer(screenObjs.miningMainContainer, miningGraphState.containers[0]);
    graphContainer(screenObjs.miningMainContainer, miningGraphState.containers[1]);  
    graphContainer(screenObjs.miningMainContainer, miningGraphState.containers[2]);

    // Add click event handlers for cycling
    lv_obj_add_event_cb(miningGraphState.containers[0], miningGraphContainerEventCallback, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(miningGraphState.containers[1], miningGraphContainerEventCallback, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(miningGraphState.containers[2], miningGraphContainerEventCallback, LV_EVENT_CLICKED, NULL);

    // Setup graphs with their content and timers
    setupHashrateGraph(miningGraphState.containers[0]);
    setupPowerGraph(miningGraphState.containers[1]);
    setupTemperatureGraph(miningGraphState.containers[2]);

    // Set up initial visibility (show hashrate graph first)
    updateMiningGraphVisibility(0);

    Serial.println("Mining stats screen with graph framework created successfully");
}
