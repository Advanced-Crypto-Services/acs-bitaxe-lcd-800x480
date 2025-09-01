#include <Arduino.h>
#include "homeScreen.h"
#include "UIScreens.h"
#include "UIThemes.h"
#include <TimeLib.h>
#include "Clock.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include "mempoolAPI.h"

#define activityScreen_MAX_VARIATIONS 3  // Change this if you want more variations per group

struct ContainerState {
    lv_obj_t* containers[activityScreen_MAX_VARIATIONS];
    int currentIndex; // Tracks which variation is currently visible
};

// Top Left Group containers
static lv_obj_t* VINContainer;
static lv_obj_t* wattageContainer;
static lv_obj_t* domainVoltageContainer;

// Top Right Group containers
static lv_obj_t* asicTempContainer;
static lv_obj_t* vRegTempContainer;
static lv_obj_t* efficiencyContainer;

// Bottom Left Group containers
static lv_obj_t* fanSpeedContainer;
static lv_obj_t* fanPercentContainer;
static lv_obj_t* asicFrequencyContainer;

// Bottom Right Group containers
static lv_obj_t* upTimeContainer;
static lv_obj_t* acceptedSharesContainer;
static lv_obj_t* rejectedSharesContainer;

// Array to hold all container states
static ContainerState containerStates[4];

static void updateDeviceStatsLabels(lv_timer_t* timer) 
{
    // Get values outside the lock
    float asicTemp = IncomingData.monitoring.temperatures[0];
    uint32_t fanSpeed = IncomingData.monitoring.fanSpeed;
    uint32_t uptime = IncomingData.monitoring.uptime;
    float voltage = IncomingData.monitoring.powerStats.voltage;
    float power = IncomingData.monitoring.powerStats.power;
    float fanPercent = IncomingData.monitoring.fanSpeedPercent;
    float vRegTemp = IncomingData.monitoring.vregTemp;
    
    // Assuming these new data sources exist in IncomingData structure
    float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage; // Assuming this exists
    float efficiency = IncomingData.mining.efficiency; // Assuming this exists
    uint32_t asicFrequency = IncomingData.monitoring.asicFrequency; // Assuming this exists
    uint32_t acceptedShares = IncomingData.mining.acceptedShares; // Assuming this exists
    uint32_t rejectedShares = IncomingData.mining.rejectedShares; // Assuming this exists
    
    // Lock for LVGL operations
    if (lvgl_port_lock(10)) 
    {  // 10ms timeout
        lv_obj_t** labels = (lv_obj_t**)timer->user_data;
        
        // Top Left Group Updates
        lv_label_set_text_fmt(labels[0], "%d.%02d V", (int)(voltage/1000), (int)(voltage)%1000); // VIN
        lv_label_set_text_fmt(labels[1], "%d.%02d W", (int)power, (int)(power * 100) % 100); // Power
        lv_label_set_text_fmt(labels[2], "%d.%02d V", (int)(domainVoltage/1000), (int)(domainVoltage)%1000); // Domain Voltage
        
        // Top Right Group Updates
        lv_label_set_text_fmt(labels[3], "%dC", (int)asicTemp); // ASIC Temp
        lv_label_set_text_fmt(labels[4], "%d C", (int)vRegTemp); // VReg Temp
        lv_label_set_text_fmt(labels[5], "%d.%01d W/TH", (int)efficiency, (int)((efficiency - (int)efficiency) * 10)); // Efficiency
        
        // Bottom Left Group Updates
        lv_label_set_text_fmt(labels[6], "%lu RPM", fanSpeed); // Fan Speed
        lv_label_set_text_fmt(labels[7], "%d %%", (int)fanPercent); // Fan Percent
        lv_label_set_text_fmt(labels[8], "%lu MHz", asicFrequency); // ASIC Frequency
        
        // Bottom Right Group Updates
        lv_label_set_text_fmt(labels[9], "%lu:%02lu:%02lu", (uptime / 3600), (uptime % 3600) / 60, uptime % 60); // Uptime
        lv_label_set_text_fmt(labels[10], "%lu", acceptedShares); // Accepted Shares
        lv_label_set_text_fmt(labels[11], "%lu", rejectedShares); // Rejected Shares
        
        lvgl_port_unlock();
    }
}

static void updateContainerVisibility(int groupIndex) {
    if (lvgl_port_lock(10)) {
        for (int i = 0; i < activityScreen_MAX_VARIATIONS; i++) {
            lv_obj_add_flag(containerStates[groupIndex].containers[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(containerStates[groupIndex].containers[containerStates[groupIndex].currentIndex], LV_OBJ_FLAG_HIDDEN);
        lvgl_port_unlock();
    }
}

static void quadContainerEventCallback(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE)) {
        for (int group = 0; group < 4; group++) {
            for (int i = 0; i < activityScreen_MAX_VARIATIONS; i++) {
                if (obj == containerStates[group].containers[i]) {
                    // Advance to next variation
                    containerStates[group].currentIndex = (containerStates[group].currentIndex + 1) % activityScreen_MAX_VARIATIONS;
                    updateContainerVisibility(group);
                    return;
                }
            }
        }
    }
}

void activityScreen() 
{
    uiTheme_t* theme = getCurrentTheme();

   // Top Left Group: VIN, Power, Domain Voltage
   quadContainer(screenObjs.activityMainContainer, VINContainer, asicTempContainer, fanSpeedContainer, upTimeContainer);
   // add clickable flags
   lv_obj_add_flag(VINContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(asicTempContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(fanSpeedContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(upTimeContainer, LV_OBJ_FLAG_CLICKABLE);
   
   // Create the first variation of each container
   quadContainerContent(VINContainer, "VOLTAGE", "SYNC");
   quadContainerContent(asicTempContainer, "ASIC TEMP", "SYNC");
   quadContainerContent(fanSpeedContainer, "FAN SPEED", "SYNC");
   quadContainerContent(upTimeContainer, "UPTIME", "SYNC");

   // Create additional containers for second and third variations
   quadContainer(screenObjs.activityMainContainer, wattageContainer, vRegTempContainer, fanPercentContainer, acceptedSharesContainer);
   lv_obj_add_flag(wattageContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(vRegTempContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(fanPercentContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(acceptedSharesContainer, LV_OBJ_FLAG_CLICKABLE);
   
   quadContainerContent(wattageContainer, "POWER", "SYNC");
   quadContainerContent(vRegTempContainer, "VREG TEMP", "SYNC");
   quadContainerContent(fanPercentContainer, "FAN PERCENT", "SYNC");
   quadContainerContent(acceptedSharesContainer, "ACCEPTED", "SYNC");
   
   // Hide second variations initially
   lv_obj_add_flag(wattageContainer, LV_OBJ_FLAG_HIDDEN);
   lv_obj_add_flag(vRegTempContainer, LV_OBJ_FLAG_HIDDEN);
   lv_obj_add_flag(fanPercentContainer, LV_OBJ_FLAG_HIDDEN);
   lv_obj_add_flag(acceptedSharesContainer, LV_OBJ_FLAG_HIDDEN);

   // Create third variations
   quadContainer(screenObjs.activityMainContainer, domainVoltageContainer, efficiencyContainer, asicFrequencyContainer, rejectedSharesContainer);
   lv_obj_add_flag(domainVoltageContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(efficiencyContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(asicFrequencyContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(rejectedSharesContainer, LV_OBJ_FLAG_CLICKABLE);
   
   quadContainerContent(domainVoltageContainer, "DOMAIN", "SYNC");
   quadContainerContent(efficiencyContainer, "EFFICIENCY", "SYNC");
   quadContainerContent(asicFrequencyContainer, "ASIC FREQ", "SYNC");
   quadContainerContent(rejectedSharesContainer, "REJECTED", "SYNC");
   
   // Hide third variations initially
   lv_obj_add_flag(domainVoltageContainer, LV_OBJ_FLAG_HIDDEN);
   lv_obj_add_flag(efficiencyContainer, LV_OBJ_FLAG_HIDDEN);
   lv_obj_add_flag(asicFrequencyContainer, LV_OBJ_FLAG_HIDDEN);
   lv_obj_add_flag(rejectedSharesContainer, LV_OBJ_FLAG_HIDDEN);

   // Array of all device stats labels for the timer update function
   static lv_obj_t* deviceStatsLabels[12] = {
       // Top Left Group (VIN, Power, Domain Voltage)
       lv_obj_get_child(VINContainer, 1),
       lv_obj_get_child(wattageContainer, 1),
       lv_obj_get_child(domainVoltageContainer, 1),
       
       // Top Right Group (ASIC Temp, VReg Temp, Efficiency)
       lv_obj_get_child(asicTempContainer, 1),
       lv_obj_get_child(vRegTempContainer, 1),
       lv_obj_get_child(efficiencyContainer, 1),
       
       // Bottom Left Group (Fan Speed, Fan Percent, ASIC Frequency)
       lv_obj_get_child(fanSpeedContainer, 1),
       lv_obj_get_child(fanPercentContainer, 1),
       lv_obj_get_child(asicFrequencyContainer, 1),
       
       // Bottom Right Group (Uptime, Accepted Shares, Rejected Shares)
       lv_obj_get_child(upTimeContainer, 1),
       lv_obj_get_child(acceptedSharesContainer, 1),
       lv_obj_get_child(rejectedSharesContainer, 1)
   };

   // Create timer to update labels every .5 seconds
   screenObjs.deviceStatsLabelsUpdateTimer = lv_timer_create(updateDeviceStatsLabels, 500, deviceStatsLabels);

   // Set up container states for 4 groups with 3 variations each
   containerStates[0] = { { VINContainer, wattageContainer, domainVoltageContainer }, 0 }; // Top Left
   containerStates[1] = { { asicTempContainer, vRegTempContainer, efficiencyContainer }, 0 }; // Top Right
   containerStates[2] = { { fanSpeedContainer, fanPercentContainer, asicFrequencyContainer }, 0 }; // Bottom Left
   containerStates[3] = { { upTimeContainer, acceptedSharesContainer, rejectedSharesContainer }, 0 }; // Bottom Right

   // Hide all but the first variation in each group
   for (int group = 0; group < 4; group++) {
       for (int i = 1; i < activityScreen_MAX_VARIATIONS; i++) {
           lv_obj_add_flag(containerStates[group].containers[i], LV_OBJ_FLAG_HIDDEN);
       }
   }

   // Add the event callback to all variations
   for (int group = 0; group < 4; group++) {
       for (int i = 0; i < activityScreen_MAX_VARIATIONS; i++) {
           lv_obj_add_event_cb(containerStates[group].containers[i], quadContainerEventCallback, LV_EVENT_CLICKED, NULL);
       }
   }
}