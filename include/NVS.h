#pragma once

#include "nvs_flash.h"
#include "nvs.h"
#include "UIThemes.h"


// Add these constants
#define NVS_NAMESPACE_THEMES "uiThemes"
#define NVS_NAMESPACE_FLAGS "uiFlags"
#define NVS_NAMESPACE_SETTINGS "uiSettings"
#define NVS_NAMESPACE_ASIC_SETTINGS "asicSettings"

#define NVS_KEY_THEME "themePreset"
#define NVS_KEY_FIRST_BOOT "firstBoot"
#define NVS_KEY_TUTORIAL_COMPLETED "tutorialDone"

#define NVS_KEY_WIFI_SSID1 "wifiSSID1"
#define NVS_KEY_WIFI_PASSWORD1 "wifiPassword1"
#define NVS_KEY_WIFI_SSID2 "wifiSSID2"
#define NVS_KEY_WIFI_PASSWORD2 "wifiPassword2"
#define NVS_KEY_BTC_ADDRESS "btcAddress"
#define NVS_KEY_TIME_OFFSET_HOURS "timeOffsetH"
#define NVS_KEY_TIME_OFFSET_MINUTES "timeOffsetM"

#define NVS_KEY_ASIC_CURRENT_FREQ "currentFq"
#define NVS_KEY_ASIC_CURRENT_VOLTAGE "currentV"
#define NVS_KEY_ASIC_CURRENT_FAN_SPEED "currentFSp"
#define NVS_KEY_ASIC_CURRENT_AUTO_FAN_SPEED "currentAFSp"
#define NVS_KEY_ASIC_AUTOTUNE_ENABLED "AutoTuneEn"
#define NVS_KEY_ASIC_PRESET_NAME "presetName"



extern void initializeNVS(void);
extern themePreset_t loadThemeFromNVS(void);
extern void saveThemeToNVS(themePreset_t preset);
extern bool isFirstBoot(void);
extern void setFirstBootComplete(void);
extern void saveSettingsToNVSasString(const char* key, const char* value, size_t size);
extern void loadSettingsFromNVSasString(const char* key, char* value, size_t size);
extern void saveSettingsToNVSasU16(const char* key, uint16_t value);
extern uint16_t loadSettingsFromNVSasU16(const char* key);
extern void factoryResetNVS(void);

