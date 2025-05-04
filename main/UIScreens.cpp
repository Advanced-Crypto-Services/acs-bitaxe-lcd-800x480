#include "UIScreens.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include <Arduino.h>
#include <SPIFFS.h>
#include "UIHandler.h"
#include "UISharedAssets.h"
#include "I2CData.h"
#include "fonts.h"
#include <TimeLib.h>
#include "BAP.h"
#include "UIThemes.h"
#include "NVS.h"
#include "mempoolAPI.h"
#include <WiFi.h>
#include "wifiFeatures.h"
#include "Clock.h"
#include "modelPresets.h"
#include "lcdSettings.h"

#define MASK_WIDTH 300
#define MASK_HEIGHT 64

// Define __min macro for use in this file
#define __min(a,b) ((a)<(b)?(a):(b))

// Tab objects
lv_obj_t* tabHome = nullptr;
lv_obj_t* tabMining = nullptr;
lv_obj_t* tabActivity = nullptr;
lv_obj_t* tabBitcoinNews = nullptr;
lv_obj_t* tabSettings = nullptr;

// Status Bar Objects
lv_obj_t* statusBarObj = nullptr;

// Screen objects
ScreenType activeScreen = activeScreenSplash; 
lv_obj_t* splashScreenContainer = nullptr;

// Timers
lv_timer_t* chartUpdateTimer = nullptr;
lv_timer_t* networkUpdateTimer = nullptr;
lv_timer_t* clockUpdateTimer = nullptr;

// Labels
lv_obj_t* miningGraphFooterPoolLabel = nullptr;
lv_obj_t* miningGraphFooterDiffLabel = nullptr;
lv_obj_t* miningGraphHashrateLabel = nullptr;
lv_obj_t* miningGraphFooterRejectRatePercentLabel = nullptr;
lv_obj_t* clockLabel = nullptr;
lv_obj_t* dateLabel = nullptr;

static lv_obj_t* lowPowerMode;
static lv_obj_t* normalPowerMode;
static lv_obj_t* highPowerMode;
static lv_obj_t* settingsWifiDiscoveredNetworksContainer;
static lv_obj_t* settingsWifiDiscoveredNetworksLabel;
static lv_obj_t* settingsWifiList;
static lv_obj_t* settingsWifiSsidTextArea;
static lv_obj_t* settingsWifiPasswordTextArea;
static lv_obj_t* settingsConfirmWifiBtn;
static lv_obj_t* settingsBacktoWifiSelectionButton;
static lv_obj_t* settingsConfirmWifiBtnLabel;   
static lv_obj_t* settingsConnectingLabel;
static lv_obj_t* networkSettingsContainer;
static lv_obj_t* timeZoneList;
static lv_obj_t* customOffsetMinutesTextArea;
static lv_obj_t* customOffsetHoursTextArea;
static lv_obj_t* timeSettingsContainer;
static lv_obj_t* timeOffsetLabel;
static lv_obj_t* customOffsetLabel;
static lv_obj_t* debugSettingsContainer;    
static lv_obj_t* targetVoltageVariableLabel;
static lv_obj_t* targetFrequencyVariableLabel;
static lv_obj_t* targetFanSpeedVariableLabel;
static lv_obj_t* offsetVoltageVariableLabel;
static lv_obj_t* offsetFrequencyVariableLabel;
static lv_obj_t* offsetFanSpeedVariableLabel;
static lv_obj_t* autotuneSwitchLabel;




static lv_obj_t* blockClockLabel;
static lv_obj_t* halvingLabel;

static bool clockContainerShown = true;
static bool blockClockContainerShown = false;
// Hashrate variables
float minHashrateSeen = 250;  // Initialize to maximum possible float

float maxHashrateSeen = 400;        // Initialize to minimum for positive values
float hashrateBuffer[SMOOTHING_WINDOW_SIZE] = {0};
int bufferIndex = 0;

// Add these as global variables to store the settings and text area references
struct {
    char hostname[32];
    char wifiSSID[MAX_SSID_LENGTH];
    char wifiPassword[64];
    char stratumUrl[128];
    char stratumPort[8];
    char stratumUser[64];
    char stratumPassword[64];
} settingsData;

SettingsTextAreas settingsTextAreas = {
    .hostnameTextArea = nullptr,
    .wifiTextArea = nullptr,
    .wifiPasswordTextArea = nullptr,
    .stratumUrlTextArea = nullptr,
    .stratumPortTextArea = nullptr,
    .stratumUserTextArea = nullptr,
    .stratumPasswordTextArea = nullptr
};

bool settingsChanged = false;

// Add near the top of the file with other function declarations
static void saveButtonEventHandler(lv_event_t* e);
static void resetAsicSettingsButtonEventHandler(lv_event_t* e);
static void checkStartupDone(lv_timer_t* timer);
static void settingsWifiListRescanButtonEventHandler(lv_event_t* e);
static void settingsConfirmWifiBtnEventHandler(lv_event_t* e);
static void settingsBacktoWifiSelectionButtonEventHandler(lv_event_t* e);
static void timeZoneListEventHandler(lv_event_t* e);
static void timeZoneSaveButtonEventHandler(lv_event_t* e);
static void overheatModeButtonEventHandler(lv_event_t* e);
static void blockFoundButtonEventHandler(lv_event_t* e);
static void dismissBlockFoundButtonEventHandler(lv_event_t* e);

lv_obj_t* themeDropdown = nullptr;
lv_obj_t* themeExampleContainer = nullptr;
lv_obj_t* themePreviewLabel = nullptr;    
lv_obj_t* themeExampleImage = nullptr;

lv_obj_t* timeZoneDropdown = nullptr;

static void themeDropdownEventHandler(lv_event_t* e);

static void factoryResetButtonEventHandler(lv_event_t* e);

static void updateAutoTuneLabels(lv_timer_t* timer) 
{
    // Get values outside the lock
    uint16_t target_voltage = currentPresetVoltage;
    uint16_t target_freq = currentPresetFrequency;
    uint8_t target_fan = currentPresetFanSpeed;
    int16_t offset_voltage = IncomingData.monitoring.targetDomainVoltage - target_voltage;
    int16_t offset_freq = IncomingData.monitoring.asicFrequency - target_freq;
    int16_t offset_fan = IncomingData.monitoring.fanSpeedPercent - target_fan;
    bool auto_fan = currentPresetAutoFanMode;
    
    
    // Lock for LVGL operations
    if (lvgl_port_lock(10)) 
    {  // 10ms timeout
        lv_obj_t** labels = (lv_obj_t**)timer->user_data;
        
        // Batch all LVGL operations together
        lv_label_set_text_fmt(labels[0], "%d mV", target_voltage);
        lv_label_set_text_fmt(labels[1], "%d MHz", target_freq);
        if(auto_fan == 0)
        { 
            lv_label_set_text_fmt(labels[2], "%d%%", target_fan);
        }
        else
        {
            lv_label_set_text(labels[2], "AUTO FANSPEED");
        }
        
        // Format offsets with signs
        lv_label_set_text_fmt(labels[3], "%s%d mV", 
            offset_voltage >= 0 ? "+" : "", offset_voltage);
        lv_label_set_text_fmt(labels[4], "%s%d MHz", 
            offset_freq >= 0 ? "+" : "", offset_freq);
        if(auto_fan == 0)
        {
        lv_label_set_text_fmt(labels[5], "%s%d%%", 
            offset_fan >= 0 ? "+" : "", offset_fan);
        }
        else
        {
            lv_label_set_text(labels[5], "AUTO FANSPEED");
        }
        
        lvgl_port_unlock();
    }
}

static void factoryResetButtonEventHandler(lv_event_t* e) {
    Serial0.println("Factory Reset Button Clicked");
    // TODO: Add a confirmation overlay
    // TODO: Set Bitaxe to factory Settings as well
    factoryResetNVS();
}

// Add the implementation before the settingsScreen function
static void saveButtonEventHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        settingsChanged = true;
        lvgl_port_lock(-1);  // Lock for LVGL operations
        
        showSettingsConfirmationOverlay();
        lvgl_port_unlock();
        delay(50);
        // Write entire buffer to BAP
        // Possibly need to change to only sending the changed settings in the future
        Serial.println("Settings changed, showing overlay");
        } else {
            Serial.println("No valid settings changes detected");
        }

        // Get values from text areas one by one

        // Hostname
        if (settingsTextAreas.hostnameTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* hostname = lv_textarea_get_text(settingsTextAreas.hostnameTextArea);
            if (hostname && strlen(hostname) > 0) {
                memset(BAPHostnameBuffer, 0, BAP_HOSTNAME_BUFFER_SIZE);
                size_t hostnameLen = __min(strlen(hostname), BAP_HOSTNAME_BUFFER_SIZE - 1);
                memcpy(BAPHostnameBuffer, hostname, hostnameLen);
                Serial.printf("Hostname: %s (length: %d)\n", hostname, hostnameLen);
                writeDataToBAP(BAPHostnameBuffer, strlen((char*)BAPHostnameBuffer), BAP_HOSTNAME_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }
        // WiFi SSID
        if (settingsTextAreas.wifiTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return; 
            }
            const char* wifiSSID = lv_textarea_get_text(settingsTextAreas.wifiTextArea);
            if (wifiSSID && strlen(wifiSSID) > 0) {
                memset(BAPWifiSSIDBuffer, 0, BAP_WIFI_SSID_BUFFER_SIZE);
                size_t ssidLen = __min(strlen(wifiSSID), BAP_WIFI_SSID_BUFFER_SIZE - 1);
                memcpy(BAPWifiSSIDBuffer, wifiSSID, ssidLen);
                Serial0.printf("WiFi SSID: %s (length: %d)\n", wifiSSID, ssidLen);
                writeDataToBAP(BAPWifiSSIDBuffer, strlen((char*)BAPWifiSSIDBuffer), BAP_WIFI_SSID_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }
        // WiFi Password
        if (settingsTextAreas.wifiPasswordTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* wifiPassword = lv_textarea_get_text(settingsTextAreas.wifiPasswordTextArea);
            if (wifiPassword && strlen(wifiPassword) > 0) {
                memset(BAPWifiPassBuffer, 0, BAP_WIFI_PASS_BUFFER_SIZE);
                size_t passLen = __min(strlen(wifiPassword), BAP_WIFI_PASS_BUFFER_SIZE - 1);
                memcpy(BAPWifiPassBuffer, wifiPassword, passLen);
                Serial.printf("WiFi Password: %s (length: %d)\n", wifiPassword, passLen);
                writeDataToBAP(BAPWifiPassBuffer, strlen((char*)BAPWifiPassBuffer), BAP_WIFI_PASS_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }
        // Stratum URL MAIN
        if (settingsTextAreas.stratumUrlTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumUrlFallback = lv_textarea_get_text(settingsTextAreas.stratumUrlTextAreaFallback);
            if (stratumUrlFallback && strlen(stratumUrlFallback) > 0) {
                memset(BAPStratumUrlFallbackBuffer, 0, BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE);
                size_t urlLen = __min(strlen(stratumUrlFallback), BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE - 1);
                memcpy(BAPStratumUrlFallbackBuffer, stratumUrlFallback, urlLen);
                Serial.printf("Stratum URL Fallback: %s (length: %d)\n", stratumUrlFallback, urlLen);
                writeDataToBAP(BAPStratumUrlFallbackBuffer, strlen((char*)BAPStratumUrlFallbackBuffer), BAP_STRATUM_URL_FALLBACK_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }
        // Stratum Port MAIN    
        if (settingsTextAreas.stratumPortTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumPort = lv_textarea_get_text(settingsTextAreas.stratumPortTextArea);
            if (stratumPort && strlen(stratumPort) > 0) {
                memset(BAPStratumPortMainBuffer, 0, BAP_STRATUM_PORT_MAIN_BUFFER_SIZE);
                uint16_t portNumber = (uint16_t)atoi(stratumPort);  // Convert string to number
                uint8_t portBytes[2] = {
                    (uint8_t)(portNumber >> 8),    // High byte
                    (uint8_t)(portNumber & 0xFF)   // Low byte
                };
                memcpy(BAPStratumPortMainBuffer, portBytes, 2);
                Serial.printf("Stratum Port: %d (length: %d)\n", portNumber, 2);
                writeDataToBAP(portBytes, 2, BAP_STRATUM_PORT_MAIN_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }

        //Stratum User MAIN
        if (settingsTextAreas.stratumUserTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumUser = lv_textarea_get_text(settingsTextAreas.stratumUserTextArea);
            if (stratumUser && strlen(stratumUser) > 0) {
                memset(BAPStratumUserMainBuffer, 0, BAP_STRATUM_USER_MAIN_BUFFER_SIZE);
                size_t userLen = __min(strlen(stratumUser), BAP_STRATUM_USER_MAIN_BUFFER_SIZE - 1);
                memcpy(BAPStratumUserMainBuffer, stratumUser, userLen);
                writeDataToBAP(BAPStratumUserMainBuffer, userLen, BAP_STRATUM_USER_MAIN_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }


        // Stratum Password MAIN
        if (settingsTextAreas.stratumPasswordTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumPassword = lv_textarea_get_text(settingsTextAreas.stratumPasswordTextArea);
            if (stratumPassword && strlen(stratumPassword) > 0) {
                memset(BAPStratumPassMainBuffer, 0, BAP_STRATUM_PASS_MAIN_BUFFER_SIZE);
                size_t passLen = __min(strlen(stratumPassword), BAP_STRATUM_PASS_MAIN_BUFFER_SIZE - 1);
                memcpy(BAPStratumPassMainBuffer, stratumPassword, passLen);
                writeDataToBAP(BAPStratumPassMainBuffer, passLen, BAP_STRATUM_PASS_MAIN_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }

        // Stratum URL Fallback
        if (settingsTextAreas.stratumUrlTextAreaFallback) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumUrlFallback = lv_textarea_get_text(settingsTextAreas.stratumUrlTextAreaFallback);
            if (stratumUrlFallback && strlen(stratumUrlFallback) > 0)
            {
                memset(BAPStratumUrlFallbackBuffer, 0, BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE);
                size_t urlLen = __min(strlen(stratumUrlFallback), BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE - 1);
                memcpy(BAPStratumUrlFallbackBuffer, stratumUrlFallback, urlLen);
                writeDataToBAP(BAPStratumUrlFallbackBuffer, strlen((char*)BAPStratumUrlFallbackBuffer), BAP_STRATUM_URL_FALLBACK_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }

        // Stratum Port Fallback
        if (settingsTextAreas.stratumPortTextAreaFallback) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumPortFallback = lv_textarea_get_text(settingsTextAreas.stratumPortTextAreaFallback);
            if (stratumPortFallback && strlen(stratumPortFallback) > 0) {
                memset(BAPStratumPortFallbackBuffer, 0, BAP_STRATUM_PORT_FALLBACK_BUFFER_SIZE);
                uint16_t portNumber = (uint16_t)atoi(stratumPortFallback);  // Convert string to number
                uint8_t portBytes[2] = {
                    (uint8_t)(portNumber >> 8),    // High byte
                    (uint8_t)(portNumber & 0xFF)   // Low byte
                };
                memcpy(BAPStratumPortFallbackBuffer, portBytes, 2);
                writeDataToBAP(portBytes, 2, BAP_STRATUM_PORT_FALLBACK_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }   

        // Stratum User Fallback
        if (settingsTextAreas.stratumUserTextAreaFallback) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumUserFallback = lv_textarea_get_text(settingsTextAreas.stratumUserTextAreaFallback);
            if (stratumUserFallback && strlen(stratumUserFallback) > 0) {
                memset(BAPStratumUserFallbackBuffer, 0, BAP_STRATUM_USER_FALLBACK_BUFFER_SIZE);
                size_t userLen = __min(strlen(stratumUserFallback), BAP_STRATUM_USER_FALLBACK_BUFFER_SIZE - 1);
                memcpy(BAPStratumUserFallbackBuffer, stratumUserFallback, userLen);
                writeDataToBAP(BAPStratumUserFallbackBuffer, strlen((char*)BAPStratumUserFallbackBuffer), BAP_STRATUM_USER_FALLBACK_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }

        // Stratum Password Fallback
        if (settingsTextAreas.stratumPasswordTextAreaFallback) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* stratumPasswordFallback = lv_textarea_get_text(settingsTextAreas.stratumPasswordTextAreaFallback);
            if (stratumPasswordFallback && strlen(stratumPasswordFallback) > 0) {
                memset(BAPStratumPassFallbackBuffer, 0, BAP_STRATUM_PASS_FALLBACK_BUFFER_SIZE);
                size_t passLen = __min(strlen(stratumPasswordFallback), BAP_STRATUM_PASS_FALLBACK_BUFFER_SIZE - 1);
                memcpy(BAPStratumPassFallbackBuffer, stratumPasswordFallback, passLen);
                writeDataToBAP(BAPStratumPassFallbackBuffer, strlen((char*)BAPStratumPassFallbackBuffer), BAP_STRATUM_PASS_FALLBACK_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }

        // ASIC Voltage
        /*
        if (settingsTextAreas.asicVoltageTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* asicVoltage = lv_textarea_get_text(settingsTextAreas.asicVoltageTextArea);
            if (asicVoltage && strlen(asicVoltage) > 0) {
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                uint16_t voltageNumber = (uint16_t)atoi(asicVoltage);  // Convert string to number
                uint8_t voltageBytes[2] = {
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                writeDataToBAP(voltageBytes, 2, BAP_ASIC_VOLTAGE_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }
        */
       if (BAPAsicVoltageBuffer != NULL && *BAPAsicVoltageBuffer != 0)
       {
        ESP_LOGI("Power Mode", "Writing ASIC Voltage to BAP");
        ESP_LOGI("Power Mode", "BAPAsicVoltageBuffer: %d", *BAPAsicVoltageBuffer);
        writeDataToBAP(BAPAsicVoltageBuffer, BAP_ASIC_VOLTAGE_BUFFER_SIZE, BAP_ASIC_VOLTAGE_BUFFER_REG);
        delay(50); // Increased delay to ensure BAP write completes
        
       saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_VOLTAGE, (uint16_t)((BAPAsicVoltageBuffer[0] << 8) | BAPAsicVoltageBuffer[1]));
       }

        // ASIC Frequency
        /*
        if (settingsTextAreas.asicFrequencyTextArea) 
        {
            if (!lvgl_port_lock(-1)) {
                Serial.println("Failed to acquire LVGL lock");
                return;
            }
            const char* asicFreq = lv_textarea_get_text(settingsTextAreas.asicFrequencyTextArea);
            if (asicFreq && strlen(asicFreq) > 0) {
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                uint16_t freqNumber = (uint16_t)atoi(asicFreq);  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
                delay(10);
            }
            lvgl_port_unlock();
        }
        */

       // Fan Speed
       if (BAPFanSpeedBuffer != NULL && BAPFanSpeedBuffer[1] != 0)
       {
        ESP_LOGI("Power Mode", "Writing Fan Speed to BAP");
        ESP_LOGI("Power Mode", "BAPFanSpeedBuffer: %d", BAPFanSpeedBuffer[1]);
        writeDataToBAP(BAPFanSpeedBuffer, BAP_FAN_SPEED_BUFFER_SIZE, BAP_FAN_SPEED_BUFFER_REG);
        saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_FAN_SPEED, (uint16_t)((BAPFanSpeedBuffer[0] << 8) | BAPFanSpeedBuffer[1]));
        delay(10);
       }
       // Auto Fan Speed
       if (BAPAutoFanSpeedBuffer != NULL && BAPAsicFreqBuffer[1] != 0) { // since auto fan speed can be 0, we tie it to another value to check if settings were changed
        ESP_LOGI("Power Mode", "Writing Auto Fan Speed to BAP");
        ESP_LOGI("Power Mode", "BAPAutoFanSpeedBuffer[1]: %d", BAPAutoFanSpeedBuffer[1]);
        writeDataToBAP(BAPAutoFanSpeedBuffer, BAP_AUTO_FAN_SPEED_BUFFER_SIZE, BAP_AUTO_FAN_SPEED_BUFFER_REG);
        saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_AUTO_FAN_SPEED, (uint16_t)((BAPAutoFanSpeedBuffer[0] << 8) | BAPAutoFanSpeedBuffer[1]));
       }
    if (BAPAsicFreqBuffer != NULL && BAPAsicFreqBuffer[1] != 0)
       {
        ESP_LOGI("Power Mode", "Writing ASIC Frequency to BAP");
        ESP_LOGI("Power Mode", "BAPAsicFreqBuffer: %d", BAPAsicFreqBuffer[1]);
        writeDataToBAP(BAPAsicFreqBuffer, BAP_ASIC_FREQ_BUFFER_SIZE, BAP_ASIC_FREQ_BUFFER_REG);
        saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_FREQ, (uint16_t)((BAPAsicFreqBuffer[0] << 8) | BAPAsicFreqBuffer[1]));
        delay(10);
       }

        // get and save theme to NVS
        uint8_t autotuneEnabledSave = autoTuneEnabled;
        saveSettingsToNVSasU16(NVS_KEY_ASIC_AUTOTUNE_ENABLED, autotuneEnabledSave);
        saveThemeToNVS(getCurrentThemePreset());
        Serial.printf("Theme saved to NVS: %d\n", getCurrentThemePreset());
        specialRegisters.restart = 1;
        
}

static void autotuneSwitchEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        bool isChecked = lv_obj_has_state(obj, LV_STATE_CHECKED);
        autoTuneEnabled = isChecked;
        ESP_LOGI("Autotune", "Switch state changed to: %s", isChecked ? "On" : "Off");
        if(isChecked)
        {
            ESP_LOGI("Autotune", "Autotune Enabled");
            lv_label_set_text(autotuneSwitchLabel, "Autotune\nEnabled");
            saveSettingsToNVSasU16(NVS_KEY_ASIC_AUTOTUNE_ENABLED, 1);
        }
        else
        {
            ESP_LOGI("Autotune", "Autotune Disabled");
            lv_label_set_text(autotuneSwitchLabel, "Autotune\nDisabled");
            saveSettingsToNVSasU16(NVS_KEY_ASIC_AUTOTUNE_ENABLED, 0);
        }
    }
}

// This is used because Timelib day function is having issues
const char* customDayStr(uint8_t day)
{ 
    static const char* const days[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (day > 7) return "";
    return days[day];
}
 
// It is used to create a gradient effect on the main container
// Not used at the moment due to performance hits
static void addMaskEventCallback(lv_event_t * e) 
{
    static lv_draw_mask_map_param_t m;
    static int16_t maskId;
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_opa_t * maskMap = (lv_opa_t *)lv_event_get_user_data(e);
    
    if(code == LV_EVENT_COVER_CHECK) 
    {
        lv_event_set_cover_res(e, LV_COVER_RES_MASKED);
    }
    else if(code == LV_EVENT_DRAW_MAIN_BEGIN) 
    {
        lv_draw_mask_map_init(&m, &obj->coords, maskMap);
        maskId = lv_draw_mask_add(&m, NULL);
    }
    else if(code == LV_EVENT_DRAW_MAIN_END) 
    {
        lv_draw_mask_free_param(&m);
        lv_draw_mask_remove_id(maskId);
    }
}

static void updateLabels(lv_timer_t* timer) 
{

    
    // Get values outside the lock
    char ssid[32], ip[32], status[32], pool[64];
    float hashrate = IncomingData.mining.hashrate;
    float efficiency = IncomingData.mining.efficiency;
    uint32_t shares = IncomingData.mining.shares;
    float temp = IncomingData.monitoring.temperatures[0];
    uint32_t acceptedShares = IncomingData.mining.acceptedShares;
    uint32_t rejectedShares = IncomingData.mining.rejectedShares;
    float rejectRatePercent = IncomingData.mining.rejectRatePercent;
    uint32_t fanSpeed = IncomingData.monitoring.fanSpeed;
    uint32_t asicFreq = IncomingData.monitoring.asicFrequency;
    uint32_t uptime = IncomingData.monitoring.uptime;
    float voltage = IncomingData.monitoring.powerStats.voltage;
    float power = IncomingData.monitoring.powerStats.power;
    float current = (voltage != 0) ? (power * 1000) / voltage : 0;
    float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage;
    
    // Lock for LVGL operations
    if (lvgl_port_lock(10)) 
    {  // 10ms timeout
        lv_obj_t** labels = (lv_obj_t**)timer->user_data;
        
        // Batch all LVGL operations together
        lv_label_set_text_fmt(labels[0], "SSID: %s", IncomingData.network.ssid);
        lv_label_set_text_fmt(labels[1], "IP: %s", IncomingData.network.ipAddress);
        lv_label_set_text_fmt(labels[2], "Status: %s", IncomingData.network.wifiStatus);
        lv_label_set_text_fmt(labels[3], "Pool 1: %s:%d\nPool 2: %s:%d", IncomingData.network.poolUrl, IncomingData.network.poolPort, IncomingData.network.fallbackUrl, IncomingData.network.fallbackPort);
        lv_label_set_text_fmt(labels[4], "Hashrate: %d.%02d GH/s", (int)hashrate, (int)(hashrate * 100) % 100);
        lv_label_set_text_fmt(labels[5], "Efficiency: %d.%02d W/TH", (int)efficiency, (int)(efficiency * 100) % 100);
        lv_label_set_text_fmt(labels[6], "Best Diff: %s", IncomingData.mining.bestDiff);
        lv_label_set_text_fmt(labels[7], "Session Diff: %s", IncomingData.mining.sessionDiff);
        lv_label_set_text_fmt(labels[8], "Accepted: %lu", acceptedShares);
        lv_label_set_text_fmt(labels[9], "Rejected: %lu", rejectedShares);
        lv_label_set_text_fmt(labels[10], "Temp: %d°C", (int)temp);
        lv_label_set_text_fmt(labels[11], "Fan: %lu RPM", fanSpeed);
        lv_label_set_text_fmt(labels[12], "Fan: %d%%", (int)IncomingData.monitoring.fanSpeedPercent);
        lv_label_set_text_fmt(labels[13], "ASIC: %lu MHz", asicFreq);
        lv_label_set_text_fmt(labels[14], "Uptime: %lu:%02lu:%02lu", (uptime / 3600), (uptime % 3600) / 60, uptime % 60);
        lv_label_set_text_fmt(labels[15], "Voltage: %d.%02d V", (int)(voltage / 1000), (int)(voltage / 10) % 100);
       // lv_label_set_text_fmt(labels[16], "Current: %d.%02d A", (int)(current / 1000), (int)(current / 10) % 100); // the TSP546 has incorrect current reading
        lv_label_set_text_fmt(labels[16], "Current: %d.%02d A", (int)current, (int)(current * 100) % 100);
        lv_label_set_text_fmt(labels[17], "Power: %d.%02d W", (int)power, (int)(power * 100) % 100);
        lv_label_set_text_fmt(labels[18], "Domain: %d mV", (int)domainVoltage);
        
        lvgl_port_unlock();
    }
}



void splashScreen()
{
    
    uiTheme_t* theme = getCurrentTheme();
    //activeScreen = activeScreenHome;
    Serial.println("Create UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    // Create a screen object
    splashScreenContainer = lv_obj_create(NULL);
    lv_scr_load(splashScreenContainer);

    // Create the Background image
    backgroundImage(splashScreenContainer);

    // Create the OSBM logo image
    lv_obj_t* osbmLogo = lv_img_create(splashScreenContainer);
    lv_img_set_src(osbmLogo, theme->logo2);
    lv_obj_center(osbmLogo);

    // Create the ACS, Bitaxe, and OSMU image
    lv_obj_t* logos = lv_img_create(splashScreenContainer);
    lv_img_set_src(logos, theme->logo1);

    lv_obj_align(logos, LV_ALIGN_BOTTOM_MID, 0, -88);
    lvgl_port_unlock();
}

static void clockContainerEventCallback(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE))
    {
        Serial.println("Clock Container Clicked");
        clockContainerShown = !clockContainerShown;
        blockClockContainerShown = !blockClockContainerShown;

        if (clockContainerShown && blockClockContainerShown)
        {
            clockContainerShown = true;
            blockClockContainerShown = false;

        }
        else if (!clockContainerShown && !blockClockContainerShown)
        {
            clockContainerShown = true;
            blockClockContainerShown = false;
        }

        if (clockContainerShown)
        {
            lvgl_port_lock(10);
            lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            lvgl_port_unlock();
        }

        else if (blockClockContainerShown)
        {
            lvgl_port_lock(10);
            lv_obj_clear_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            lvgl_port_unlock();
        }

            

    }
}



void homeScreen() 
{
    uiTheme_t* theme = getCurrentTheme();
    activeScreen = activeScreenHome;
    /*
    // Not using Logos on the home screen at the moment.
    lv_obj_t* logos = lv_img_create(screenObjs.homeMainContainer);
    lv_img_set_src(logos, "S:/Logos.png");
    lv_obj_align(logos, LV_ALIGN_TOP_MID, 0, 0);
    lv_img_set_zoom(logos, 384);
    //lv_obj_align(mainContainerLabel, LV_ALIGN_CENTER, 0, 0);  // Initial alignment 
    //lv_obj_add_style(mainContainerLabel, &mainContainerBorder, 0);
   // lvgl_port_unlock();
    */

    // Create the clock container
    lv_obj_t* clockContainer = lv_obj_create(screenObjs.homeMainContainer);
    lv_obj_set_size(clockContainer, 672, 304);
    lv_obj_align(clockContainer, LV_ALIGN_CENTER, 0, -48);
    lv_obj_set_style_bg_opa(clockContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(clockContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(clockContainer, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_border_width(clockContainer, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(clockContainer, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_add_flag(clockContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(clockContainer, clockContainerEventCallback, LV_EVENT_CLICKED, NULL);
    
    // Create the clock label
    clockLabel = lv_label_create(clockContainer);
    lv_label_set_text(clockLabel, "-- : --");
    lv_obj_set_style_text_font(clockLabel, theme->fontExtraBold144, LV_PART_MAIN);
    lv_obj_set_style_text_color(clockLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(clockLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(clockLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    //lv_obj_set_style_border_width(clockLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(clockLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);

    // Create the date label
    dateLabel = lv_label_create(clockContainer);
    lv_label_set_text(dateLabel, "Syncing . . .");
    lv_obj_set_style_text_font(dateLabel, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(dateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(dateLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(dateLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(dateLabel, LV_ALIGN_BOTTOM_MID, 0, 16);
    //lv_obj_set_style_border_width(dateLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(dateLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);

    // create block clock in clock container, hidden by default
    blockClockLabel = lv_label_create(clockContainer);
    lv_label_set_text(blockClockLabel, "Block Height:\n --");
    lv_obj_set_style_text_font(blockClockLabel, theme->fontExtraBold72, LV_PART_MAIN);
    lv_obj_set_style_text_color(blockClockLabel, theme->textColor, LV_PART_MAIN);

    lv_obj_align(blockClockLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(blockClockLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_set_style_border_width(clockLabel, 1, LV_PART_MAIN);

    //lv_obj_set_style_border_color(clockLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);

    // Create the halving label
    halvingLabel = lv_label_create(clockContainer);
    lv_label_set_text(halvingLabel, "Syncing . . .");
    lv_obj_set_style_text_font(halvingLabel, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(halvingLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(halvingLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(halvingLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(halvingLabel, LV_ALIGN_BOTTOM_MID, 0, 24);
    lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_set_style_border_width(dateLabel, 1, LV_PART_MAIN);


    //lv_obj_set_style_border_color(dateLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);


    

    // Create the clock timer
    screenObjs.clockTimer = lv_timer_create([](lv_timer_t* timer) 
    {
        if (clockContainerShown)
        {
            
            // Get values outside the lock
            uint8_t h = hourFormat12();
            uint8_t m = minute();
            uint8_t s = second();
            uint8_t mo = month();
            uint8_t d = day();


        uint8_t w = weekday();
        uint16_t y = year();    
        bool isAm = isAM();

        // Lock for LVGL operations
        if (lvgl_port_lock(10)) 
        {  // 10ms timeout
            //lv_obj_clear_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_clear_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            


            // Check if the time is before 2000, This is used to check if the time has been set
            if (now() < 946684800) 
            {
                Serial.println("Time is before 2000");
                lv_label_set_text(dateLabel, "Syncing . . .");
            }
            else 
            {
                // Batch all LVGL operations together
                lv_label_set_text_fmt(clockLabel, "%2d:%02d%s", h, m, isAm ? "AM" : "PM");
                lv_label_set_text_fmt(dateLabel, "%s\n%s %d %d", customDayStr(w), monthStr(mo), d, y);
            }
            
            lvgl_port_unlock();
        }
        }
        else if (blockClockContainerShown)
        {
            


            // Get values outside the lock
            MempoolApiState* mempoolState = getMempoolState();
            uint32_t blockHeight = mempoolState->blockHeight;
            uint32_t blockToHalving = 1050000 - blockHeight;


        // Lock for LVGL operations
        if (lvgl_port_lock(10)) 
        {  // 10ms timeout
            //lv_obj_clear_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
           // lv_obj_clear_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(blockClockLabel, "Block Height:\n %lu", blockHeight);
            lv_label_set_text_fmt(halvingLabel, "Halving In:\n %lu Blocks", blockToHalving);
            

            lvgl_port_unlock();

        }
        }
        

    }, 1000, &clockLabel);
}


void activityScreen() 
{
    uiTheme_t* theme = getCurrentTheme();
    activeScreen = activeScreenActivity;
    //lvgl_port_lock(-1);

    //This is used for Gradient 
    /*static lv_opa_t maskMap[MASK_WIDTH * MASK_HEIGHT];
    // Create canvas for mask
    lv_obj_t * canvas = lv_canvas_create(screenObjs.activityMainContainer);
    lv_canvas_set_buffer(canvas, maskMap, MASK_WIDTH, MASK_HEIGHT, LV_IMG_CF_ALPHA_8BIT);
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_TRANSP);

    // Draw text to canvas
    lv_draw_label_dsc_t labelDsc;
    lv_draw_label_dsc_init(&labelDsc);
    labelDsc.color = lv_color_white();
    labelDsc.font = &interExtraBold56;
    labelDsc.align = LV_TEXT_ALIGN_CENTER;
    //lv_canvas_draw_text(canvas, 5, 5, MASK_WIDTH, &labelDsc, "10 : 00AM");

    // Delete canvas after creating mask
    lv_obj_del(canvas);
    */ 

    // Not using Logos on the activity screen at the moment.
    /*
    lv_obj_t* logos = lv_img_create(screenObjs.activityMainContainer);
    lv_img_set_src(logos, "S:/Logos.png");
    lv_obj_align(logos, LV_ALIGN_TOP_MID, 0, 0);
    lv_img_set_zoom(logos, 384);
    */

    /*
    // Create gradient object
    lv_obj_t * gradObj = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(gradObj, MASK_WIDTH, MASK_HEIGHT);
    lv_obj_center(gradObj);
    lv_obj_set_style_bg_color(gradObj, lv_color_hex(0x34D399), 0);
    lv_obj_set_style_bg_grad_color(gradObj, lv_color_hex(0xA7F3D0), 0);
    lv_obj_set_style_border_width(gradObj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(gradObj, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_bg_grad_dir(gradObj, LV_GRAD_DIR_VER, 0);
    //lv_obj_add_event_cb(gradObj, addMaskEventCallback, LV_EVENT_ALL, maskMap);
    */

    // Create container for network info
    lv_obj_t* networkContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(networkContainer, 360, 184);
    lv_obj_align(networkContainer, LV_ALIGN_TOP_LEFT, -24, 0);
    lv_obj_set_style_bg_opa(networkContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(networkContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(networkContainer, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_border_width(networkContainer, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(networkContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    Serial.println("Network Container Created");

    //Network Container Label
    lv_obj_t* networkContainerLabel = lv_label_create(networkContainer);
    lv_label_set_text(networkContainerLabel, "Network");
    lv_obj_set_style_text_font(networkContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(networkContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(networkContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Network Container Label Created");
    // SSID Label
    lv_obj_t* ssidLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(ssidLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ssidLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(ssidLabel, "SSID: %s", IncomingData.network.ssid);
    lv_obj_align(ssidLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("SSID Label Created");
    // IP Address Label
    lv_obj_t* ipLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(ipLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ipLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(ipLabel, "IP: %s", IncomingData.network.ipAddress);
    lv_obj_align(ipLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("IP Address Label Created");
    // WiFi Status Label
    lv_obj_t* wifiStatusLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(wifiStatusLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifiStatusLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(wifiStatusLabel, "Status: %s", IncomingData.network.wifiStatus);
    lv_obj_align(wifiStatusLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("WiFi Status Label Created");
    // Pool Info Label
    lv_obj_t* poolUrlLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(poolUrlLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(poolUrlLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(poolUrlLabel, "Pool 1: %s:%d\nPool 2: %s:%d", IncomingData.network.poolUrl, IncomingData.network.poolPort, IncomingData.network.fallbackUrl, IncomingData.network.fallbackPort);
    lv_obj_align(poolUrlLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("Pool Info Label Created");

   

    // Create container for mining info
    lv_obj_t* miningContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(miningContainer, 304, 184);
    lv_obj_align(miningContainer, LV_ALIGN_TOP_RIGHT, 16, 0);
    lv_obj_set_style_bg_opa(miningContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(miningContainer, 0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(miningContainer, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(miningContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_clear_flag(miningContainer, LV_OBJ_FLAG_SCROLLABLE);
    Serial.println("Mining Container Created");

    // Mining Container Label
    lv_obj_t* miningContainerLabel = lv_label_create(miningContainer);
    lv_label_set_text(miningContainerLabel, "Mining");
    lv_obj_set_style_text_font(miningContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(miningContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(miningContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Mining Container Label Created");
    // Hashrate Label
    lv_obj_t* hashrateLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(hashrateLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(hashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(hashrateLabel, "Hashrate: %d.%02d GH/s", (int)IncomingData.mining.hashrate, (int)(IncomingData.mining.hashrate * 100) % 100);
    lv_obj_align(hashrateLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("Hashrate Label Created");
    // Efficiency Label
    lv_obj_t* efficiencyLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(efficiencyLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(efficiencyLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(efficiencyLabel, "Efficiency: %d.%02d W/TH", (int)IncomingData.mining.efficiency, (int)(IncomingData.mining.efficiency * 100) % 100);
    lv_obj_align(efficiencyLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("Efficiency Label Created");
    // Best Difficulty Label
    lv_obj_t* bestDiffLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(bestDiffLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(bestDiffLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(bestDiffLabel, "Best Diff: %s", IncomingData.mining.bestDiff);
    lv_obj_align(bestDiffLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("Best Difficulty Label Created");
    // Session Difficulty Label
    lv_obj_t* sessionDiffLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(sessionDiffLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(sessionDiffLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(sessionDiffLabel, "Session Diff: %s", IncomingData.mining.sessionDiff);
    lv_obj_align(sessionDiffLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("Session Difficulty Label Created");

    /*// Shares Label
    lv_obj_t* sharesLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(sharesLabel, &interMedium16_19px, LV_PART_MAIN);
    lv_obj_set_style_text_color(sharesLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_label_set_text_fmt(sharesLabel, "Shares: %u", IncomingData.mining.shares);
    lv_obj_align(sharesLabel, LV_ALIGN_TOP_LEFT, 16, 104);
    Serial.println("Shares Label Created");*/
   
    //accepted shares label
    lv_obj_t* acceptedSharesLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(acceptedSharesLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(acceptedSharesLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(acceptedSharesLabel, "Accepted Shares: %u", IncomingData.mining.acceptedShares);
    lv_obj_align(acceptedSharesLabel, LV_ALIGN_TOP_LEFT, 16, 104);
    Serial.println("Accepted Shares Label Created");
    //rejected shares label
    lv_obj_t* rejectedSharesLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(rejectedSharesLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(rejectedSharesLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(rejectedSharesLabel, "Rejected Shares: %u", IncomingData.mining.rejectedShares);
    lv_obj_align(rejectedSharesLabel, LV_ALIGN_TOP_LEFT, 16, 128);
    Serial.println("Rejected Shares Label Created");

    /*// rejected shares percent label
    lv_obj_t* rejectedSharesPercentLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(rejectedSharesPercentLabel, &interMedium16_19px, LV_PART_MAIN);
    lv_obj_set_style_text_color(rejectedSharesPercentLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_label_set_text_fmt(rejectedSharesPercentLabel, "Reject Rate: %d.%02d%%", (int)IncomingData.mining.rejectRatePercent, (int)(IncomingData.mining.rejectRatePercent * 100) % 100);
    lv_obj_align(rejectedSharesPercentLabel, LV_ALIGN_TOP_LEFT, 16, 176);
    Serial.println("Rejected Shares Percent Label Created");*/


    // Monitoring Container 
    lv_obj_t* monitoringContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(monitoringContainer, 304, 152);
    lv_obj_align(monitoringContainer, LV_ALIGN_BOTTOM_RIGHT, 16, 0);
    Serial.println("Monitoring Container Created");
    lv_obj_set_style_bg_opa(monitoringContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(monitoringContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(monitoringContainer, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_border_color(monitoringContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);

    // Monitoring Container Label
    lv_obj_t* monitoringContainerLabel = lv_label_create(monitoringContainer);
    lv_label_set_text(monitoringContainerLabel, "Monitoring");
    lv_obj_set_style_text_font(monitoringContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(monitoringContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(monitoringContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Monitoring Container Label Created");
    // Temperature Label
    lv_obj_t* tempLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(tempLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(tempLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(tempLabel, "Temp: %d°C", (int)IncomingData.monitoring.temperatures[0]);
    lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("Temperature Label Created");
    // Fan Speed Label
    lv_obj_t* fanSpeedLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(fanSpeedLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(fanSpeedLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(fanSpeedLabel, "Fan: %f RPM", IncomingData.monitoring.fanSpeed);
    lv_obj_align(fanSpeedLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("Fan Speed Label Created");
    // Fan Speed Percent Label
    lv_obj_t* fanSpeedPercentLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(fanSpeedPercentLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(fanSpeedPercentLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(fanSpeedPercentLabel, "Fan: %d%%", (int)IncomingData.monitoring.fanSpeedPercent);
    lv_obj_align(fanSpeedPercentLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("Fan Speed Percent Label Created");
    // asic Frequency Label
    lv_obj_t* asicFreqLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(asicFreqLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(asicFreqLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(asicFreqLabel, "ASIC: %lu MHz", IncomingData.monitoring.asicFrequency);
    lv_obj_align(asicFreqLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("ASIC Frequency Label Created");
    // uptime label
    lv_obj_t* uptimeLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(uptimeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(uptimeLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(uptimeLabel, "Uptime: %lu:%02lu:%02lu", (IncomingData.monitoring.uptime / 3600), (IncomingData.monitoring.uptime % 3600) / 60, IncomingData.monitoring.uptime % 60);
    lv_obj_align(uptimeLabel, LV_ALIGN_TOP_LEFT, 16, 104);
    Serial.println("Uptime Label Created");


    // Power Container
    lv_obj_t* powerContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(powerContainer, 360, 152);
    lv_obj_align(powerContainer, LV_ALIGN_BOTTOM_LEFT, -24, 0);
    lv_obj_set_style_bg_opa(powerContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(powerContainer, 0, LV_PART_MAIN);
    //lv_obj_set_style_border_color(powerContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);    
    Serial.println("Power Container Created");
    // Power Container Label
    lv_obj_t* powerContainerLabel = lv_label_create(powerContainer);
    lv_label_set_text(powerContainerLabel, "Power");
    lv_obj_set_style_text_font(powerContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(powerContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(powerContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Power Container Label Created");
    // voltage label
    lv_obj_t* voltageLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(voltageLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(voltageLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(voltageLabel, "Voltage: %d.%02d V", (int)(IncomingData.monitoring.powerStats.voltage / 1000), (int)(IncomingData.monitoring.powerStats.voltage / 10) % 100);
    lv_obj_align(voltageLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("Voltage Label Created");
    // current label
    lv_obj_t* currentLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(currentLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(currentLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(currentLabel, "Current: 0 A");
    lv_obj_align(currentLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("Current Label Created");
    // Power Label
    lv_obj_t* powerLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(powerLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(powerLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(powerLabel, "Power: %d.%02d W", (int)IncomingData.monitoring.powerStats.power, (int)(IncomingData.monitoring.powerStats.power * 100) % 100);
    lv_obj_align(powerLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("Power Label Created");
    // Domain Voltage Label
    lv_obj_t* domainVoltageLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(domainVoltageLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(domainVoltageLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(domainVoltageLabel, "Asic Voltage: %d mV", (int)IncomingData.monitoring.powerStats.domainVoltage);
    lv_obj_align(domainVoltageLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("Domain Voltage Label Created");
    



     static lv_obj_t* allLabels[19] = 
    {
        ssidLabel, ipLabel, wifiStatusLabel, poolUrlLabel,
        hashrateLabel, efficiencyLabel, 
        bestDiffLabel, sessionDiffLabel, acceptedSharesLabel, rejectedSharesLabel,
        tempLabel, fanSpeedLabel, fanSpeedPercentLabel, asicFreqLabel, uptimeLabel, voltageLabel, currentLabel, powerLabel, domainVoltageLabel
    };
    screenObjs.labelUpdateTimer = lv_timer_create(updateLabels, 2000, allLabels);

    //lv_obj_align(mainContainerLabel, LV_ALIGN_CENTER, 0, 0);  // Initial alignment 
    //lv_obj_add_style(mainContainerLabel, &mainContainerBorder, 0);
   // lvgl_port_unlock();


}

/*void miningStatusScreen() //TODO: Remove this screen
{
    activeScreen = activeScreenMining;
    //lvgl_port_lock(-1);
    lv_obj_t* miningStatusScreen = lv_obj_create(NULL);
    lv_scr_load_anim(miningStatusScreen, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, true);

    backgroundImage(miningStatusScreen);
    lvglTabIcons(miningStatusScreen);
    statusBar(miningStatusScreen);
    mainContainer(miningStatusScreen);

  hashrateGraph(screenObjs.miningMainContainer);
  

}*/



void initalizeOneScreen() 
{
    lvgl_port_lock(-1);
    activeScreen = activeScreenHome;

    // Initialize timer pointers to NULL
    screenObjs.labelUpdateTimer = NULL;
    screenObjs.chartUpdateTimer = NULL;
    screenObjs.clockTimer = NULL;
    screenObjs.apiUpdateTimer = NULL;

    Serial.println("Timer Pointers Initialized");
    // Create and load the background screen
    screenObjs.background = lv_obj_create(NULL);
    lv_scr_load(screenObjs.background);
    Serial.println("Background Screen Created");
    // Create basic UI elements
    backgroundImage(screenObjs.background);
    Serial.println("Background Image Created");
    lvglTabIcons(screenObjs.background);
    Serial.println("Tab Icons Created");
    statusBar(screenObjs.background);
    Serial.println("Status Bar Created");
    
    // Create containers but keep them hidden initially
    mainContainer(screenObjs.background);
    screenObjs.homeMainContainer = lv_obj_get_child(screenObjs.background, -1);  // Get last created object
    lv_obj_add_flag(screenObjs.homeMainContainer, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Home Container Created");
    mainContainer(screenObjs.background);
    screenObjs.miningMainContainer = lv_obj_get_child(screenObjs.background, -1);  // Get last created object
    lv_obj_add_flag(screenObjs.miningMainContainer, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Mining Container Created");

    mainContainer(screenObjs.background);
    screenObjs.activityMainContainer = lv_obj_get_child(screenObjs.background, -1);  // Get last created object
    lv_obj_add_flag(screenObjs.activityMainContainer, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Activity Container Created");

    mainContainer(screenObjs.background);
    screenObjs.bitcoinNewsMainContainer = lv_obj_get_child(screenObjs.background, -1);  // Get last created object
    lv_obj_add_flag(screenObjs.bitcoinNewsMainContainer, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Bitcoin News Container Created");

    mainContainer(screenObjs.background);
    screenObjs.settingsMainContainer = lv_obj_get_child(screenObjs.background, -1);  // Get last created object
    lv_obj_add_flag(screenObjs.settingsMainContainer, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Settings Container Created");
    
    // Initialize screens without timers
    activityScreen();
    homeScreen();
    Serial.println("Home Screen Created");
    hashrateGraph(screenObjs.miningMainContainer);
    Serial.println("Hashrate Graph Created");
    bitcoinNewsScreen();
    Serial.println("Bitcoin News Screen Created");
    settingsScreen();
    Serial.println("Settings Screen Created");
    // Now show the initial screen and start its timer
    lv_obj_clear_flag(screenObjs.homeMainContainer, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Home Screen Shown");
    // Create timers only after all objects are initialized
    if (screenObjs.labelUpdateTimer) 
    {
        lv_timer_pause(screenObjs.labelUpdateTimer);
        Serial.println("Label Update Timer Paused");
    }
    if (screenObjs.chartUpdateTimer) 
    {
        lv_timer_pause(screenObjs.chartUpdateTimer);
        Serial.println("Chart Update Timer Paused");
    }
    if (screenObjs.clockTimer) 
    {
        lv_timer_pause(screenObjs.clockTimer);
        Serial.println("Clock Timer Paused");
        }
    // Start home screen timer
    lv_timer_resume(screenObjs.clockTimer);
    Serial.println("Clock Timer Resumed");
    
    lvgl_port_unlock();
    Serial.println("UI Initialized");
}

// Create a helper function to calculate moving average
float calculateMovingAverage(float newValue) 
{
    hashrateBuffer[bufferIndex] = newValue;
    bufferIndex = (bufferIndex + 1) % SMOOTHING_WINDOW_SIZE;
    
    float sum = 0;
    for (int i = 0; i < SMOOTHING_WINDOW_SIZE; i++) 
    {
        sum += hashrateBuffer[i];
    }
    return sum / SMOOTHING_WINDOW_SIZE;
}

void hashrateGraph(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    // Pool Footer Label
    miningGraphFooterPoolLabel = lv_label_create(parent);
    lv_label_set_text_fmt(miningGraphFooterPoolLabel, "Pool: %s", IncomingData.network.poolUrl);
    lv_obj_set_style_text_font(miningGraphFooterPoolLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(miningGraphFooterPoolLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(miningGraphFooterPoolLabel, 320);
    lv_label_set_long_mode(miningGraphFooterPoolLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(miningGraphFooterPoolLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(miningGraphFooterPoolLabel, LV_OPA_100, LV_PART_MAIN);
    //lv_obj_set_style_border_width(miningGraphFooterPoolLabel, 1, LV_PART_MAIN);
   // lv_obj_set_style_border_color(miningGraphFooterPoolLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(miningGraphFooterPoolLabel, LV_ALIGN_BOTTOM_LEFT,-16, 0);

    // Diff Footer Label
    miningGraphFooterDiffLabel = lv_label_create(parent);
    lv_label_set_text_fmt(miningGraphFooterDiffLabel, "Difficulty: %s", IncomingData.mining.sessionDiff);
    lv_obj_set_style_text_font(miningGraphFooterDiffLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(miningGraphFooterDiffLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(miningGraphFooterDiffLabel, 152);
    lv_label_set_long_mode(miningGraphFooterDiffLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(miningGraphFooterDiffLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(miningGraphFooterDiffLabel, LV_OPA_100, LV_PART_MAIN);
    //lv_obj_set_style_border_width(miningGraphFooterDiffLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(miningGraphFooterDiffLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(miningGraphFooterDiffLabel, LV_ALIGN_BOTTOM_MID, 72, 0);

    // Reject Rate Footer Label
    miningGraphFooterRejectRatePercentLabel = lv_label_create(parent);
    lv_label_set_text_fmt(miningGraphFooterRejectRatePercentLabel, "Reject Rate: %d.%02d%%", (int)IncomingData.mining.rejectRatePercent, (int)(IncomingData.mining.rejectRatePercent * 100) % 100);
    lv_obj_set_style_text_font(miningGraphFooterRejectRatePercentLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(miningGraphFooterRejectRatePercentLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(miningGraphFooterRejectRatePercentLabel, 280);
    lv_label_set_long_mode(miningGraphFooterRejectRatePercentLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(miningGraphFooterRejectRatePercentLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(miningGraphFooterRejectRatePercentLabel, LV_OPA_100, LV_PART_MAIN);
   // lv_obj_set_style_border_width(miningGraphFooterRejectRatePercentLabel, 1, LV_PART_MAIN);
   // lv_obj_set_style_border_color(miningGraphFooterRejectRatePercentLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(miningGraphFooterRejectRatePercentLabel, LV_ALIGN_BOTTOM_MID, 296, 0);

    // Hashrate Chart Label
    lv_obj_t* hashrateChartLabel = lv_label_create(parent);
    lv_label_set_text(hashrateChartLabel, "Hashrate");
    lv_obj_set_style_text_font(hashrateChartLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(hashrateChartLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(hashrateChartLabel, LV_ALIGN_TOP_LEFT, 24, -16);

     // Hashrate Label
    miningGraphHashrateLabel = lv_label_create(parent);
    lv_label_set_text_fmt(miningGraphHashrateLabel, "%d.%02d GH/s", (int)IncomingData.mining.hashrate, (int)(IncomingData.mining.hashrate * 100) % 100);
    lv_obj_set_style_text_font(miningGraphHashrateLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_align(miningGraphHashrateLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(miningGraphHashrateLabel, 296);
    lv_label_set_long_mode(miningGraphHashrateLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(miningGraphHashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(miningGraphHashrateLabel, LV_OPA_100, LV_PART_MAIN);
    //lv_obj_set_style_border_width(miningGraphFooterHashrateLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(miningGraphFooterHashrateLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(miningGraphHashrateLabel, LV_ALIGN_TOP_LEFT, 136, -16);

    lv_obj_t* miningStatusHashRateChart = lv_chart_create(parent);
    lv_obj_set_size(miningStatusHashRateChart, 648, 280);
    lv_obj_align(miningStatusHashRateChart, LV_ALIGN_CENTER, 32, -16);
    lv_obj_set_style_bg_opa(miningStatusHashRateChart, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_side(miningStatusHashRateChart, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_width(miningStatusHashRateChart, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(miningStatusHashRateChart, theme->borderColor, LV_PART_MAIN);
    lv_chart_set_type(miningStatusHashRateChart, LV_CHART_TYPE_LINE);

    // Add these style configurations
    lv_obj_set_style_line_width(miningStatusHashRateChart, 4, LV_PART_ITEMS);    // Make lines thicker
    lv_obj_set_style_size(miningStatusHashRateChart, 0, LV_PART_INDICATOR);      // Make points bigger
    //lv_obj_set_style_radius(miningStatusHashRateChart, 24, LV_PART_INDICATOR);    // Make points rounder

    // Optional: Add drop shadow to make it look even better
    //lv_obj_set_style_shadow_width(miningStatusHashRateChart, 32, LV_PART_ITEMS);
    //lv_obj_set_style_shadow_color(miningStatusHashRateChart, lv_color_hex(0xA7F3D0), LV_PART_ITEMS);
    //lv_obj_set_style_shadow_opa(miningStatusHashRateChart, LV_OPA_30, LV_PART_ITEMS);

    // Style the grid lines
    //lv_obj_set_style_line_color(miningStatusHashRateChart, lv_color_hex(0xBBBBBB), LV_PART_MAIN);  // Grid color
    //lv_obj_set_style_line_width(miningStatusHashRateChart, 4, LV_PART_MAIN);  // Grid line thickness
    lv_obj_set_style_line_opa(miningStatusHashRateChart, LV_OPA_TRANSP, LV_PART_MAIN);  // Grid line opacity
    //lv_obj_set_style_shadow_width(miningStatusHashRateChart, 12, LV_PART_MAIN);
    //lv_obj_set_style_shadow_color(miningStatusHashRateChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    //lv_obj_set_style_shadow_opa(miningStatusHashRateChart, LV_OPA_30, LV_PART_MAIN);

    // Set chart properties with buffer of 100 on each end
    float baseHashrate = abs(IncomingData.mining.hashrate);
    //lv_chart_set_range(miningStatusHashRateChart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);

    // Set the number of points to display
    lv_chart_set_point_count(miningStatusHashRateChart, 50);  // How many points to display

    // Add series
    lv_chart_series_t* hashRateSeries = lv_chart_add_series(miningStatusHashRateChart, theme->primaryColor, LV_CHART_AXIS_PRIMARY_Y);

    // Only add initial points if we have valid data

        for(int i = 0; i < 50; i++) 
        {
            lv_chart_set_next_value(miningStatusHashRateChart, hashRateSeries, IncomingData.mining.hashrate);
        }

    // Update timer to use actual hashrate
    screenObjs.chartUpdateTimer = lv_timer_create([](lv_timer_t* timer) 
    {
        // Get values outside the lock
        float currentHashrate = IncomingData.mining.hashrate;
        char poolUrl[128];
        uint16_t poolPort = IncomingData.network.poolPort;
        strlcpy(poolUrl, IncomingData.network.poolUrl, sizeof(poolUrl));
        
        // Calculate derived values
        float smoothedHashrate = calculateMovingAverage(currentHashrate);
        float rejectRatePercent = IncomingData.mining.rejectRatePercent;
        // Lock for LVGL operations
        if (lvgl_port_lock(10)) 
        {  // 10ms timeout
            lv_obj_t* chart = (lv_obj_t*)timer->user_data;
            lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);
            
            // Batch all LVGL operations together
            lv_label_set_text_fmt(miningGraphFooterPoolLabel, "Pool: %s:%d", poolUrl, poolPort);
            lv_label_set_text_fmt(miningGraphHashrateLabel, "%d.%02d GH/s", (int)currentHashrate, (int)(currentHashrate * 100) % 100);
            lv_label_set_text_fmt(miningGraphFooterRejectRatePercentLabel, "Reject Rate: %d.%02d%%", (int)rejectRatePercent, (int)(rejectRatePercent * 100) % 100);
            lv_label_set_text_fmt(miningGraphFooterDiffLabel, "Difficulty: %s", IncomingData.mining.sessionDiff);
            
            if (smoothedHashrate < minHashrateSeen || smoothedHashrate > maxHashrateSeen) {
                if (smoothedHashrate < minHashrateSeen) minHashrateSeen = smoothedHashrate;
                if (smoothedHashrate > maxHashrateSeen) maxHashrateSeen = smoothedHashrate;
                
                lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                    (minHashrateSeen > 100) ? minHashrateSeen - 100 : 0,
                    maxHashrateSeen + 100);
            }
            
            lv_chart_set_next_value(chart, series, smoothedHashrate);
            lvgl_port_unlock();
        }
    }, 2000, miningStatusHashRateChart);

    // Optional: Add axis labels
    lv_chart_set_axis_tick(miningStatusHashRateChart, LV_CHART_AXIS_PRIMARY_Y, 5, 5, 6, 4, true, 50);
    lv_obj_clear_flag(miningStatusHashRateChart, LV_OBJ_FLAG_SCROLLABLE);
    //lvgl_port_unlock();

}

static void updateMempoolAPIInfo(lv_timer_t* timer) 
{
    auto* labels = (lv_obj_t**)timer->user_data;
    
    // Get mempool state pointer
    MempoolApiState* mempoolState = getMempoolState();
    
    // Get values from mempool state
    uint32_t btcPriceUSD = mempoolState->priceUSD;
    double networkHashrate = mempoolState->networkHashrate;
    double networkDifficulty = mempoolState->networkDifficulty;
    uint32_t blockHeight = mempoolState->blockHeight;
    uint32_t remainingTimeToDifficultyAdjustment = mempoolState->remainingTimeToDifficultyAdjustment;
    uint32_t remainingBlocksToDifficultyAdjustment = mempoolState->remainingBlocksToDifficultyAdjustment;
    double difficultyProgressPercent = mempoolState->difficultyProgressPercent;
    double difficultyChangePercent = mempoolState->difficultyChangePercent;
    uint32_t fastestFee = mempoolState->fastestFee;
    uint32_t halfHourFee = mempoolState->halfHourFee;
    uint32_t hourFee = mempoolState->hourFee;
    uint32_t economyFee = mempoolState->economyFee;
    uint32_t minimumFee = mempoolState->minimumFee;

    if (lvgl_port_lock(10)) 
    {
        lv_label_set_text_fmt(labels[0], "$%d,%03d", (int)(btcPriceUSD/1000), (int)(btcPriceUSD) % 1000);
        lv_label_set_text_fmt(labels[1], "Hashrate: %d.%02d EH/s", (int)(networkHashrate / 1e18), (int)(networkHashrate / 1e18) % 100);
        lv_label_set_text_fmt(labels[2], "Difficulty: %d.%02d T", (int)(networkDifficulty / 1e12), (int)(networkDifficulty / 1e12) % 100);
        lv_label_set_text_fmt(labels[3], "Block Height: %lu", blockHeight);
        lv_label_set_text_fmt(labels[4], "Blocks to Halving: %lu", 1050000 - blockHeight);
        lv_bar_set_value(labels[5], (int)(difficultyProgressPercent), LV_ANIM_OFF);
        lv_label_set_text_fmt(labels[6], "PROGRESS: %d.%02d%%\nREMAINING BLOCKS: %lu\nREMAINING TIME: %lu d:%02lu h\nESTIMATED CHANGE: %s%d.%02d%%", 
        (int)(difficultyProgressPercent), 
        (int)(difficultyProgressPercent * 100) % 100,
        remainingBlocksToDifficultyAdjustment,
        remainingTimeToDifficultyAdjustment / 86400, 
        (remainingTimeToDifficultyAdjustment / 3600) % 24,
        // Add %s in format string for the sign
        difficultyChangePercent < 0 ? "-" : "",
        // Use abs() for the whole number
        abs((int)difficultyChangePercent),
        // Use abs() for the decimal part
        abs((int)(difficultyChangePercent * 100) % 100)
        );
        lv_label_set_text_fmt(labels[7], "Fastest Fee: %lu SAT/B", fastestFee);
        lv_label_set_text_fmt(labels[8], "Half Hour Fee: %lu SAT/B", halfHourFee);
        lv_label_set_text_fmt(labels[9], "Hour Fee: %lu SAT/B", hourFee);
        lv_label_set_text_fmt(labels[10], "Economy Fee: %lu SAT/B", economyFee);
        lv_label_set_text_fmt(labels[11], "Minimum Fee: %lu SAT/B", minimumFee);
        lvgl_port_unlock();
    }
}

void bitcoinNewsScreen()
{
    uiTheme_t* theme = getCurrentTheme();
    activeScreen = activeScreenBitcoinNews;

    lv_obj_t* btcPriceContainer = lv_obj_create(screenObjs.bitcoinNewsMainContainer);
    lv_obj_set_size(btcPriceContainer, 320, 192 );
    lv_obj_align(btcPriceContainer, LV_ALIGN_TOP_LEFT, -16, -16);
    lv_obj_set_style_bg_opa(btcPriceContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btcPriceContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(btcPriceContainer, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(btcPriceContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);

    // BTC Price Label
    lv_obj_t* btcPriceLabel = lv_label_create(btcPriceContainer);
    lv_label_set_text(btcPriceLabel, "BTC PRICE");
    lv_obj_set_style_text_font(btcPriceLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcPriceLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcPriceLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(btcPriceContainer, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Price Value Label
    lv_obj_t* btcPriceValueLabel = lv_label_create(btcPriceContainer);
    lv_label_set_text_fmt(btcPriceValueLabel, "$%d,%03d", (int)(IncomingData.api.btcPriceUSD/1000), (int)(IncomingData.api.btcPriceUSD) % 1000);
    lv_obj_set_style_text_font(btcPriceValueLabel, theme->fontExtraBold56, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcPriceValueLabel, theme->primaryColor, LV_PART_MAIN);
    lv_obj_align(btcPriceValueLabel, LV_ALIGN_TOP_LEFT, 0, 56);

    // BTC Hashrate and Difficulty Container
    lv_obj_t* btcHashrateDiffContainer = lv_obj_create(screenObjs.bitcoinNewsMainContainer);
    lv_obj_set_size(btcHashrateDiffContainer, 320, 192);
    lv_obj_align(btcHashrateDiffContainer, LV_ALIGN_TOP_RIGHT, 16, -16);
    lv_obj_set_style_bg_opa(btcHashrateDiffContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btcHashrateDiffContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(btcHashrateDiffContainer, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(btcHashrateDiffContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);

    // BTC Network Hashrate and Difficulty Container Label
    lv_obj_t* btcNetworkMiningContainerLabel = lv_label_create(btcHashrateDiffContainer);
    lv_label_set_text(btcNetworkMiningContainerLabel, "BTC NETWORK");
    lv_obj_set_style_text_font(btcNetworkMiningContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcNetworkMiningContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcNetworkMiningContainerLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(btcNetworkMiningContainerLabel, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Network Hashrate Label
    lv_obj_t* btcNetworkHashrateLabel = lv_label_create(btcHashrateDiffContainer);
    lv_label_set_text_fmt(btcNetworkHashrateLabel, "Hashrate: %d.%02d EH/s", (int)(IncomingData.api.networkHashrate / 1e18), (int)(IncomingData.api.networkHashrate / 1e18) % 100);
    lv_obj_set_style_text_font(btcNetworkHashrateLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcNetworkHashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcNetworkHashrateLabel, LV_ALIGN_TOP_LEFT, 0, 32);

    // BTC Network Difficulty Label
    lv_obj_t* btcNetworkDifficultyLabel = lv_label_create(btcHashrateDiffContainer);
    lv_label_set_text_fmt(btcNetworkDifficultyLabel, "Difficulty: %d.%02d T", (int)(IncomingData.api.networkDifficulty / 1e12), (int)(IncomingData.api.networkDifficulty / 1e12) % 100);
    lv_obj_set_style_text_font(btcNetworkDifficultyLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcNetworkDifficultyLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcNetworkDifficultyLabel, LV_ALIGN_TOP_LEFT, 0, 56);

    // BTC Block Height Label
    lv_obj_t* btcBlockHeightLabel = lv_label_create(btcHashrateDiffContainer);
    lv_label_set_text_fmt(btcBlockHeightLabel, "Block Height: %lu", IncomingData.api.blockHeight);
    lv_obj_set_style_text_font(btcBlockHeightLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcBlockHeightLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcBlockHeightLabel, LV_ALIGN_TOP_LEFT, 0, 80);

    // BTC Remaining Blocks to Halving Label
    lv_obj_t* btcRemainingBlocksToHalvingLabel = lv_label_create(btcHashrateDiffContainer);
    lv_label_set_text_fmt(btcRemainingBlocksToHalvingLabel, "Blocks to Halving: %lu", 1050000 - IncomingData.api.blockHeight);
    lv_obj_set_style_text_font(btcRemainingBlocksToHalvingLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcRemainingBlocksToHalvingLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcRemainingBlocksToHalvingLabel, LV_ALIGN_TOP_LEFT, 0, 104);

    // BTC Difficulty Stats Container
    lv_obj_t* btcDifficultyStatsContainer = lv_obj_create(screenObjs.bitcoinNewsMainContainer);
    lv_obj_set_size(btcDifficultyStatsContainer, 320, 184);
    lv_obj_align(btcDifficultyStatsContainer, LV_ALIGN_BOTTOM_LEFT, -16, 16);
    lv_obj_set_style_bg_opa(btcDifficultyStatsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(btcDifficultyStatsContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btcDifficultyStatsContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_color(btcDifficultyStatsContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_clear_flag(btcDifficultyStatsContainer, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Difficulty Progress Label
    lv_obj_t* btcDifficultyProgressLabel = lv_label_create(btcDifficultyStatsContainer);
    lv_label_set_text(btcDifficultyProgressLabel, "DIFFICULTY PROGRESS");
    lv_obj_set_style_text_font(btcDifficultyProgressLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcDifficultyProgressLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(btcDifficultyProgressLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align(btcDifficultyProgressLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(btcDifficultyProgressLabel, LV_OBJ_FLAG_SCROLLABLE);

    //Difficulty Progress Bar
    static lv_style_t styleBarBG;
    static lv_style_t styleBarIndicator;
    lv_style_init(&styleBarIndicator);
    //lv_style_set_radius(&styleBar, LV_RADIUS_CIRCLE);
    lv_style_set_border_color(&styleBarIndicator, theme->borderColor);
    lv_style_set_bg_opa(&styleBarIndicator, LV_OPA_70);
    lv_style_set_bg_color(&styleBarIndicator, theme->primaryColor);
    lv_style_set_radius(&styleBarIndicator, 8);

    lv_style_init(&styleBarBG);
    lv_style_set_border_color(&styleBarBG, theme->borderColor);
    lv_style_set_bg_opa(&styleBarBG, LV_OPA_30);
    lv_style_set_bg_color(&styleBarBG, theme->primaryColor);
    lv_style_set_radius(&styleBarBG, 12);

    lv_obj_t* btcDifficultyProgressBar = lv_bar_create(btcDifficultyStatsContainer);
    lv_obj_set_size(btcDifficultyProgressBar, 304, 144);
    lv_obj_align(btcDifficultyProgressBar, LV_ALIGN_TOP_LEFT, -16, 16);
    lv_obj_add_style(btcDifficultyProgressBar, &styleBarBG, LV_PART_MAIN);
    lv_obj_add_style(btcDifficultyProgressBar, &styleBarIndicator, LV_PART_INDICATOR);
    lv_bar_set_value(btcDifficultyProgressBar, (int)(IncomingData.api.difficultyProgressPercent), LV_ANIM_OFF);
    //lv_obj_set_style_bg_opa(btcDifficultyProgressBar, LV_OPA_80, LV_PART_MAIN);
    
    // BTC Difficulty Progress Percent Label
    lv_obj_t* btcDifficultyProgressPercentLabel = lv_label_create(btcDifficultyProgressBar);
    lv_label_set_text_fmt(btcDifficultyProgressPercentLabel, "PROGRESS: %d.%02d%%\nREMAINING BLOCKS: %lu\nREMAINING TIME: %lu:%02luh\nESTIMATED CHANGE: %s%d.%02d%%", 
        (int)(IncomingData.api.difficultyProgressPercent), 
        (int)(IncomingData.api.difficultyProgressPercent * 100) % 100,
        IncomingData.api.remainingBlocksToDifficultyAdjustment,
        IncomingData.api.remainingTimeToDifficultyAdjustment / 86400, 
        (IncomingData.api.remainingTimeToDifficultyAdjustment / 3600) % 24,
        // Add %s in format string for the sign
        IncomingData.api.difficultyChangePercent < 0 ? "-" : "",
        // Use abs() for the whole number
        abs((int)IncomingData.api.difficultyChangePercent),
        // Use abs() for the decimal part
        abs((int)(IncomingData.api.difficultyChangePercent * 100) % 100)
    );
    
    lv_obj_set_style_text_font(btcDifficultyProgressPercentLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcDifficultyProgressPercentLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_align(btcDifficultyProgressPercentLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(btcDifficultyProgressPercentLabel, LV_OBJ_FLAG_SCROLLABLE);
    lv_label_set_long_mode(btcDifficultyProgressPercentLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(btcDifficultyProgressPercentLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(btcDifficultyProgressPercentLabel, 288);
    //lv_obj_set_style_border_width(btcDifficultyProgressPercentLabel, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(btcDifficultyProgressPercentLabel, lv_color_hex(0x000000), LV_PART_MAIN);

    // BTC Fees Container
    lv_obj_t* btcFeesContainer = lv_obj_create(screenObjs.bitcoinNewsMainContainer);
    lv_obj_set_size(btcFeesContainer, 320, 184);
    lv_obj_align(btcFeesContainer, LV_ALIGN_BOTTOM_RIGHT, 16, 16);
    lv_obj_set_style_bg_opa(btcFeesContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(btcFeesContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btcFeesContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_color(btcFeesContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_clear_flag(btcFeesContainer, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Fees Label
    lv_obj_t* btcFeesLabel = lv_label_create(btcFeesContainer);
    lv_label_set_text(btcFeesLabel, "NETWORK FEES");
    lv_obj_set_style_text_font(btcFeesLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcFeesLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcFeesLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(btcFeesLabel, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Fastest Fee Label
    lv_obj_t* btcFastestFeeLabel = lv_label_create(btcFeesContainer);
    lv_label_set_text_fmt(btcFastestFeeLabel, "Fastest Fee: %d SAT/B", (int)(IncomingData.api.fastestFee));
    lv_obj_set_style_text_font(btcFastestFeeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcFastestFeeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcFastestFeeLabel, LV_ALIGN_TOP_LEFT, 0, 32);
    lv_obj_clear_flag(btcFastestFeeLabel, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Half Hour Fee Label
    lv_obj_t* btcHalfHourFeeLabel = lv_label_create(btcFeesContainer);
    lv_label_set_text_fmt(btcHalfHourFeeLabel, "Half Hour Fee: %d SAT/B", (int)(IncomingData.api.halfHourFee));
    lv_obj_set_style_text_font(btcHalfHourFeeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcHalfHourFeeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcHalfHourFeeLabel, LV_ALIGN_TOP_LEFT, 0, 56);
    lv_obj_clear_flag(btcHalfHourFeeLabel, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Hour Fee Label
    lv_obj_t* btcHourFeeLabel = lv_label_create(btcFeesContainer);
    lv_label_set_text_fmt(btcHourFeeLabel, "Hour Fee: %d SAT/B", (int)(IncomingData.api.hourFee));
    lv_obj_set_style_text_font(btcHourFeeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcHourFeeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcHourFeeLabel, LV_ALIGN_TOP_LEFT, 0, 80);
    lv_obj_clear_flag(btcHourFeeLabel, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Economy Fee Label   
    lv_obj_t* btcEconomyFeeLabel = lv_label_create(btcFeesContainer);
    lv_label_set_text_fmt(btcEconomyFeeLabel, "Economy Fee: %d SAT/B", (int)(IncomingData.api.economyFee));
    lv_obj_set_style_text_font(btcEconomyFeeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcEconomyFeeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcEconomyFeeLabel, LV_ALIGN_TOP_LEFT, 0, 104);
    lv_obj_clear_flag(btcEconomyFeeLabel, LV_OBJ_FLAG_SCROLLABLE);

    // BTC Minimum Fee Label
    lv_obj_t* btcMinimumFeeLabel = lv_label_create(btcFeesContainer);
    lv_label_set_text_fmt(btcMinimumFeeLabel, "Minimum Fee: %d SAT/B", (int)(IncomingData.api.minimumFee));
    lv_obj_set_style_text_font(btcMinimumFeeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcMinimumFeeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcMinimumFeeLabel, LV_ALIGN_TOP_LEFT, 0, 128);
    lv_obj_clear_flag(btcMinimumFeeLabel, LV_OBJ_FLAG_SCROLLABLE);

    static lv_obj_t* mempoolAPILabels[12]; // Adjust size as needed
    mempoolAPILabels[0] = btcPriceValueLabel;
    mempoolAPILabels[1] = btcNetworkHashrateLabel;
    mempoolAPILabels[2] = btcNetworkDifficultyLabel;
    mempoolAPILabels[3] = btcBlockHeightLabel;
    mempoolAPILabels[4] = btcRemainingBlocksToHalvingLabel;
    mempoolAPILabels[5] = btcDifficultyProgressBar;
    mempoolAPILabels[6] = btcDifficultyProgressPercentLabel;
    mempoolAPILabels[7] = btcFastestFeeLabel;
    mempoolAPILabels[8] = btcHalfHourFeeLabel;
    mempoolAPILabels[9] = btcHourFeeLabel;
    mempoolAPILabels[10] = btcEconomyFeeLabel;
    mempoolAPILabels[11] = btcMinimumFeeLabel;

    screenObjs.apiUpdateTimer = lv_timer_create(updateMempoolAPIInfo, 3000, mempoolAPILabels);
}
static lv_obj_t * kb;
static void ta_event_cb(lv_event_t * e);
 static void setCursorStyles(lv_obj_t * ta);
 static lv_obj_t* setTextAreaStyles(lv_obj_t* parent, const char* placeholder);

 static lv_obj_t* setTextAreaStyles(lv_obj_t* parent, const char* placeholder)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* ta = lv_textarea_create(parent);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_obj_set_style_text_color(ta, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_font(ta, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ta, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ta, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(ta, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(ta, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_radius(ta, 16, LV_PART_MAIN);
    // lv_textarea_set_max_length(ta, 32); // This should be set per text area
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_set_width(ta, lv_pct(80));
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_text_color(ta, theme->textColor, LV_PART_CURSOR);
    lv_obj_clear_flag(ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(ta, LV_OPA_0, LV_PART_SELECTED);
    return ta;
}

 static void setCursorStyles(lv_obj_t * ta)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_set_style_bg_color(ta, theme->textColor, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(ta, LV_OPA_0, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(ta, 1, LV_PART_CURSOR | LV_STATE_FOCUSED);  // Remove border completely
    lv_obj_set_style_border_color(ta, theme->textColor, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_border_side(ta, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR | LV_STATE_FOCUSED);
}


// Add this callback function outside the main function:
static void power_mode_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            // Uncheck other checkboxes
            if(obj != lowPowerMode) lv_obj_clear_state(lowPowerMode, LV_STATE_CHECKED);
            if(obj != normalPowerMode) lv_obj_clear_state(normalPowerMode, LV_STATE_CHECKED);
            if(obj != highPowerMode) lv_obj_clear_state(highPowerMode, LV_STATE_CHECKED);
            
            // Handle power mode change
            if(obj == lowPowerMode) {
                setLowPowerPreset();
            } else if(obj == normalPowerMode) {
                setNormalPowerPreset();
            } else if(obj == highPowerMode) {
                setHighPowerPreset();
            }
        } else {
            // Prevent unchecking the last checked checkbox
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        }
    }
}

static char settingsTempWifiSSID[64 + 1];
static char settingsTempWifiPassword[64 + 1];

static void settingsWifiListEventHandler(lv_event_t* e) {
    lv_obj_t* list = lv_event_get_target(e);
    lv_obj_t* selectedBtn = lv_event_get_target(e);
    const char* selectedSsid = lv_list_get_btn_text(list, selectedBtn);
    strncpy(settingsTempWifiSSID, selectedSsid, sizeof(settingsTempWifiSSID) - 1);
    settingsTempWifiSSID[sizeof(settingsTempWifiSSID) - 1] = '\0'; // Ensure null termination
/*
    lv_obj_add_flag(settingsWifiList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(settingsWifiSsidTextArea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifiPasswordTextArea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(confirmWifiBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(backtoWifiSelectionButton, LV_OBJ_FLAG_HIDDEN);
*/
    lv_textarea_set_text(settingsWifiSsidTextArea, settingsTempWifiSSID);

Serial0.println(settingsTempWifiSSID);
}

static void settingsConfirmWifiBtnEventHandler(lv_event_t* e) {
    Serial0.println("settingsConfirmWifiBtnEventHandler");
    //get theme
     uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* btn = lv_event_get_target(e);
    Serial0.println("confirmWifiBtnEventHandler");

    // delete existing connecting label if it exists
    lvgl_port_lock(-1);

    if (settingsConnectingLabel != NULL) {
        lv_obj_del(settingsConnectingLabel);
    }
    lv_refr_now(NULL);
    lvgl_port_unlock();

    //create connecting label
    lvgl_port_lock(-1);
    settingsConnectingLabel = lv_label_create(networkSettingsContainer);
    lv_obj_set_style_text_font(settingsConnectingLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(settingsConnectingLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(settingsConnectingLabel, LV_ALIGN_BOTTOM_RIGHT, -32, -16);
    lv_obj_set_style_text_align(settingsConnectingLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(settingsConnectingLabel, "Testing connection");
    delay(100);
    lv_refr_now(NULL);
    lvgl_port_unlock();
    delay(100);

     // Get credentials from text areas
    strncpy(settingsTempWifiSSID, lv_textarea_get_text(settingsWifiSsidTextArea), sizeof(settingsTempWifiSSID) - 1);
    settingsTempWifiSSID[sizeof(settingsTempWifiSSID) - 1] = '\0';
    strncpy(settingsTempWifiPassword, lv_textarea_get_text(settingsWifiPasswordTextArea), sizeof(settingsTempWifiPassword) - 1);
    settingsTempWifiPassword[sizeof(settingsTempWifiPassword) - 1] = '\0';
    
    //test connection
    Serial0.printf("SSID: %s\n", settingsTempWifiSSID);
    Serial0.printf("Password: %s\n", settingsTempWifiPassword);

    WiFi.begin(settingsTempWifiSSID, settingsTempWifiPassword);
    Serial0.printf("Connecting to %s\n", settingsTempWifiSSID);
    unsigned long currentTime = millis();
    while (millis() - currentTime < 10000) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial0.printf("Connected to %s\n", settingsTempWifiSSID);
            
            break;
        }
        delay(100);
        Serial0.printf(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial0.printf("Failed to connect to %s\n", settingsTempWifiSSID);
        
    }

    if (WiFi.status() == WL_CONNECTED) {
        lv_label_set_text(settingsConnectingLabel, "Connection successful");

        // Save to NVS
        saveSettingsToNVSasString(NVS_KEY_WIFI_SSID1, settingsTempWifiSSID, strlen(settingsTempWifiSSID));
        saveSettingsToNVSasString(NVS_KEY_WIFI_PASSWORD1, settingsTempWifiPassword, strlen(settingsTempWifiPassword));

        // Send to BAP Port
        writeDataToBAP((uint8_t*)settingsTempWifiSSID, strlen(settingsTempWifiSSID), BAP_WIFI_SSID_BUFFER_REG);
        delay(500);
        writeDataToBAP((uint8_t*)settingsTempWifiPassword, strlen(settingsTempWifiPassword), BAP_WIFI_PASS_BUFFER_REG);
        delay(20);
        // Restart the Bitaxe
        specialRegisters.restart = 1;

    }
    else {
        lv_label_set_text(settingsConnectingLabel, "Connection failed\n Check Password");
    }


}

static void settingsWifiListRescanButtonEventHandler(lv_event_t* e) {
    uiTheme_t* theme = getCurrentTheme();
    Serial0.println("settingsWifiListRescanButtonEventHandler");
       // Clear existing items from the list
    lv_obj_clean(settingsWifiList);
    lv_refr_now(NULL);
    delay(100);

    // Ensure WiFi is properly disconnected and in the correct mode
    WiFi.disconnect(true);  // true = disable WiFi altogether
    delay(20);
    
    WiFi.mode(WIFI_OFF);    // Completely disable WiFi
    delay(20);
    
    WiFi.mode(WIFI_STA);    // Re-enable WiFi in station mode
    delay(20);
    
    // Now safe to scan
    listNetworks();  // Scan for networks
    
    // Repopulate list with newly scanned networks
    if (storedNetworks != nullptr && *storedNetworkCount > 0) {
        for (uint16_t i = 0; i < *storedNetworkCount; i++) {
            // Add list item with WiFi icon and SSID
            lv_obj_t* btn = lv_list_add_btn(settingsWifiList, 
                "S:/wifi40x40.png", 
                storedNetworks[i].ssid);
                
            // Style the button with increased height and padding
            lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(btn, theme->textColor, LV_PART_MAIN);
            //lv_obj_set_style_min_height(btn, 56, LV_PART_MAIN);  // Increase button height
            //lv_obj_set_style_pad_top(btn, 24, LV_PART_MAIN);     // Add padding
            //lv_obj_set_style_pad_bottom(btn, 24, LV_PART_MAIN);
            lv_obj_set_style_text_font(btn, theme->fontMedium24, LV_PART_MAIN); // Increase font size
            lv_obj_add_event_cb(btn, settingsWifiListEventHandler, LV_EVENT_CLICKED, NULL);

            // recolor the wifi icon
            lv_obj_t* img = lv_obj_get_child(btn, 0);  // Get the image child
            lv_obj_set_style_img_recolor(img, theme->primaryColor, LV_PART_MAIN);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        }
    } else {
        // Add a message if no networks are found
        lv_list_add_text(settingsWifiList, "No networks found");
    }
}

void settingsScreen()
{
    uiTheme_t* theme = getCurrentTheme();
    activeScreen = activeScreenSettings;


    lv_obj_t * settingTabView = lv_tabview_create(screenObjs.settingsMainContainer, LV_DIR_BOTTOM, 72);
    // Make tileview transparent
    lv_obj_set_style_bg_opa(settingTabView, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(settingTabView, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(lv_tabview_get_content(settingTabView), LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_set_size(settingTabView, 672, 392);
    lv_obj_align(settingTabView, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_opa(settingTabView, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(settingTabView, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(settingTabView, theme->borderColor, LV_PART_MAIN);
    lv_obj_clear_flag(settingTabView, LV_OBJ_FLAG_SCROLLABLE);


    lv_obj_t * networkSettingsTab = lv_tabview_add_tab(settingTabView, "WIFI");
    lv_obj_clear_flag(networkSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * screenSettingsTab = lv_tabview_add_tab(settingTabView, "SCREEN");
    lv_obj_clear_flag(screenSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * asicSettingsTab = lv_tabview_add_tab(settingTabView, "PRESET");
    lv_obj_clear_flag(asicSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * autoTuneSettingsTab = lv_tabview_add_tab(settingTabView, "TUNE");
    lv_obj_clear_flag(autoTuneSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * themeSettingsTab = lv_tabview_add_tab(settingTabView, "THEME");
    lv_obj_clear_flag(themeSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * timeSettingsTab = lv_tabview_add_tab(settingTabView, "TIME");
    lv_obj_clear_flag(timeSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    #if (DEBUG_UI == 1)
    lv_obj_t * debugSettingsTab = lv_tabview_add_tab(settingTabView, "DEBUG");
    lv_obj_clear_flag(debugSettingsTab, LV_OBJ_FLAG_SCROLLABLE);
    #endif
    lv_obj_t * saveSettingsTab = lv_tabview_add_tab(settingTabView, "SAVE");

    lv_obj_clear_flag(saveSettingsTab, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(settingTabView);
    lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_0, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_0, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_30, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_set_style_border_color(tab_btns, theme->borderColor, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_radius(tab_btns, 16, LV_PART_ITEMS);
    // Style for both checked and unchecked states
    lv_obj_set_style_text_font(tab_btns, theme->fontMedium16, LV_PART_ITEMS);  // Default state
    lv_obj_set_style_text_font(tab_btns, theme->fontMedium16, LV_PART_ITEMS | LV_STATE_CHECKED);  // Checked state
    lv_obj_set_style_text_color(tab_btns, theme->textColor, LV_PART_ITEMS);  // Default state
    lv_obj_set_style_text_color(tab_btns, theme->textColor, LV_PART_ITEMS | LV_STATE_CHECKED);  // Checked state

    // Make individual tile transparent (optional if tileview is already transparent)
    lv_obj_set_style_bg_opa(networkSettingsTab, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(networkSettingsTab, LV_OPA_0, LV_PART_MAIN);


    // Network Settings Container
    networkSettingsContainer = lv_obj_create(networkSettingsTab);
    lv_obj_set_size(networkSettingsContainer, 672, 312);
    lv_obj_align(networkSettingsContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(networkSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(networkSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(networkSettingsContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Network Settings Label
    lv_obj_t* networkSettingsLabel = lv_label_create(networkSettingsContainer);
    lv_label_set_text(networkSettingsLabel, "NETWORK SETTINGS");
    lv_obj_set_style_text_font(networkSettingsLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(networkSettingsLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(networkSettingsLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(networkSettingsLabel, LV_OBJ_FLAG_SCROLLABLE);

/*
    // Hostname Text Area
    lv_obj_t* hostnameTextArea = setTextAreaStyles(networkSettingsContainer, "Hostname");
    lv_obj_align(hostnameTextArea, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_set_width(hostnameTextArea, lv_pct(40));
    lv_obj_add_event_cb(hostnameTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(hostnameTextArea, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(hostnameTextArea);

    // Wifi SSID Text Area
    lv_obj_t* wifiTextArea = setTextAreaStyles(networkSettingsContainer, "Network SSID");
    lv_textarea_set_max_length(wifiTextArea, MAX_SSID_LENGTH);
    lv_obj_align(wifiTextArea, LV_ALIGN_TOP_LEFT, 0, 80);
    lv_obj_set_width(wifiTextArea, lv_pct(40));
    lv_obj_add_event_cb(wifiTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(wifiTextArea, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(wifiTextArea);

    // Wifi Password Text Area
    lv_obj_t* wifiPasswordTextArea = setTextAreaStyles(networkSettingsContainer, "Network Password");
    lv_textarea_set_password_mode(wifiPasswordTextArea, true);
    lv_obj_align(wifiPasswordTextArea, LV_ALIGN_TOP_LEFT, 0, 144);
    lv_obj_set_width(wifiPasswordTextArea, lv_pct(40));
    lv_obj_add_event_cb(wifiPasswordTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(wifiPasswordTextArea, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(wifiPasswordTextArea);
*/

    //wifi discovered networks container
    settingsWifiDiscoveredNetworksContainer = lv_obj_create(networkSettingsContainer);
    lv_obj_set_size(settingsWifiDiscoveredNetworksContainer, 320, 264);
    lv_obj_align(settingsWifiDiscoveredNetworksContainer, LV_ALIGN_LEFT_MID, 0, 16);
    lv_obj_set_style_bg_color(settingsWifiDiscoveredNetworksContainer, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settingsWifiDiscoveredNetworksContainer, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(settingsWifiDiscoveredNetworksContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(settingsWifiDiscoveredNetworksContainer, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_opa(settingsWifiDiscoveredNetworksContainer, LV_OPA_100, LV_PART_MAIN);
/*
        //wifi discovered networks label
    lv_obj_t* settingsWifiDiscoveredNetworksLabel = lv_label_create(settingsWifiDiscoveredNetworksContainer);
    lv_label_set_text(settingsWifiDiscoveredNetworksLabel, "Wifi");
    lv_obj_set_style_text_font(settingsWifiDiscoveredNetworksLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(settingsWifiDiscoveredNetworksLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(settingsWifiDiscoveredNetworksLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(settingsWifiDiscoveredNetworksLabel, LV_ALIGN_TOP_MID, 0, 24);
    Serial0.println("settingsWifiDiscoveredNetworksLabel");

    // wifi icon
    lv_obj_t* settingsWifiIcon = lv_img_create(settingsWifiDiscoveredNetworksContainer);
     lv_img_set_src(settingsWifiIcon, "S:/wifi40x40.png");
    lv_obj_set_style_bg_opa(settingsWifiIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(settingsWifiIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(settingsWifiIcon, LV_OPA_COVER, LV_PART_MAIN);
    //lv_img_set_zoom(wifiIcon, 512);
    lv_obj_align(settingsWifiIcon, LV_ALIGN_TOP_MID, 0, -24);
    Serial0.println("settingsWifiIcon Created");
*/

    settingsWifiList = lv_list_create(settingsWifiDiscoveredNetworksContainer);
    lv_obj_set_size(settingsWifiList, 296, 240);
    lv_obj_align(settingsWifiList, LV_ALIGN_CENTER, 0, 8);
    lv_obj_set_style_bg_opa(settingsWifiList, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(settingsWifiList, 0, LV_PART_MAIN);
    Serial0.println("settingsWifiList Created");

    // Populate list with stored networks
    if (storedNetworks != nullptr && *storedNetworkCount > 0) {
        for (uint16_t i = 0; i < *storedNetworkCount; i++) {
            // Add list item with WiFi icon and SSID
            lv_obj_t* btn = lv_list_add_btn(settingsWifiList, 
                "S:/wifi40x40.png", 
                storedNetworks[i].ssid);
                
            // Style the button with increased height and padding
            lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(btn, theme->textColor, LV_PART_MAIN);
            //lv_obj_set_style_min_height(btn, 56, LV_PART_MAIN);  // Increase button height
            //lv_obj_set_style_pad_top(btn, 24, LV_PART_MAIN);     // Add padding
            //lv_obj_set_style_pad_bottom(btn, 24, LV_PART_MAIN);
            lv_obj_set_style_text_font(btn, theme->fontMedium24, LV_PART_MAIN); // Increase font size
            lv_obj_add_event_cb(btn, settingsWifiListEventHandler, LV_EVENT_CLICKED, NULL);

             // Add recoloring for the WiFi icon
            lv_obj_t* img = lv_obj_get_child(btn, 0);  // Get the image child
            lv_obj_set_style_img_recolor(img, theme->primaryColor, LV_PART_MAIN);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        }
    } else {
        // Add a message if no networks are found
        lv_list_add_text(settingsWifiList, "No networks found");
    }
    Serial0.println("settingsWifiList Populated");
    // clear scrollable flag
    lv_obj_clear_flag(settingsWifiDiscoveredNetworksContainer, LV_OBJ_FLAG_SCROLLABLE);
    Serial0.println("initSetupWifiScreen");

     // Rescan Button
    lv_obj_t* rescanButton = lv_btn_create(networkSettingsContainer);
    lv_obj_set_size(rescanButton, 128, 48);
    lv_obj_align(rescanButton, LV_ALIGN_TOP_RIGHT, -144, 144);
    lv_obj_set_style_bg_color(rescanButton, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rescanButton, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_flag(rescanButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(rescanButton, settingsWifiListRescanButtonEventHandler, LV_EVENT_CLICKED, NULL);

    //rescan button label
    lv_obj_t* rescanButtonLabel = lv_label_create(rescanButton);
    lv_label_set_text(rescanButtonLabel, "Rescan");
    lv_obj_set_style_text_font(rescanButtonLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(rescanButtonLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(rescanButtonLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_center(rescanButtonLabel);
    

    
    // add hidden text fields for wifi ssid and password after chosen from the list
    settingsWifiSsidTextArea = setTextAreaStyles(networkSettingsContainer, "SSID");
    setCursorStyles(settingsWifiSsidTextArea);
    lv_obj_set_size(settingsWifiSsidTextArea, 276, 48);
    lv_obj_align(settingsWifiSsidTextArea, LV_ALIGN_TOP_RIGHT, 0, 16);
    lv_obj_set_style_bg_color(settingsWifiSsidTextArea, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settingsWifiSsidTextArea, LV_OPA_80, LV_PART_MAIN);
    //lv_obj_add_flag(settingsWifiSsidTextArea, LV_OBJ_FLAG_HIDDEN);
    Serial0.println("settingsWifiSsidTextArea Created");
    lv_obj_add_event_cb(settingsWifiSsidTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    settingsWifiPasswordTextArea = setTextAreaStyles(networkSettingsContainer, "Password");
    setCursorStyles(settingsWifiPasswordTextArea);
    lv_obj_set_size(settingsWifiPasswordTextArea, 276, 48);
    lv_obj_align(settingsWifiPasswordTextArea, LV_ALIGN_TOP_RIGHT, 0, 80);
    lv_obj_set_style_bg_color(settingsWifiPasswordTextArea, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settingsWifiPasswordTextArea, LV_OPA_80, LV_PART_MAIN);
    //lv_obj_add_flag(settingsWifiPasswordTextArea, LV_OBJ_FLAG_HIDDEN);
    Serial0.println("settingsWifiPasswordTextArea Created");

    // add a button to confirm the wifi setup
    settingsConfirmWifiBtn = lv_btn_create(networkSettingsContainer);
    lv_obj_set_size(settingsConfirmWifiBtn, 128, 48);
    lv_obj_align(settingsConfirmWifiBtn, LV_ALIGN_TOP_RIGHT, 0, 144);
    lv_obj_set_style_bg_color(settingsConfirmWifiBtn, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settingsConfirmWifiBtn, LV_OPA_100, LV_PART_MAIN);
    //lv_obj_add_flag(settingsConfirmWifiBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t* settingsConfirmWifiBtnLabel = lv_label_create(settingsConfirmWifiBtn);
    lv_obj_add_event_cb(settingsConfirmWifiBtn, settingsConfirmWifiBtnEventHandler, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(settingsConfirmWifiBtnLabel, "Confirm");
    lv_obj_set_style_text_font(settingsConfirmWifiBtnLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(settingsConfirmWifiBtnLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(settingsConfirmWifiBtnLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_center(settingsConfirmWifiBtnLabel);

    // Screen Settings Container
    lv_obj_t* screenSettingsContainer = lv_obj_create(screenSettingsTab);
    lv_obj_set_size(screenSettingsContainer, 672, 312);
    lv_obj_align(screenSettingsContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(screenSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(screenSettingsContainer, LV_OPA_0, LV_PART_MAIN);

    // Screen Settings Label
    lv_obj_t* screenSettingsLabel = lv_label_create(screenSettingsContainer);
    lv_label_set_text(screenSettingsLabel, "SCREEN SETTINGS");
    lv_obj_set_style_text_font(screenSettingsLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(screenSettingsLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(screenSettingsLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(screenSettingsLabel, LV_OBJ_FLAG_SCROLLABLE);

    // Screen Brightness Slider styles
    static lv_style_t style_main;
    static lv_style_t style_indicator;
    static lv_style_t style_knob;
    static lv_style_t style_pressed_color;
    //lv_obj_set_style_bg_color(screenBrightnessSlider, theme->backgroundColor, LV_PART_MAIN);
    //lv_obj_set_style_bg_opa(screenBrightnessSlider, LV_OPA_80, LV_PART_MAIN);
    //lv_obj_set_style_border_width(screenBrightnessSlider, 0, LV_PART_MAIN);
    // main style
    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, theme->backgroundColor);
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    // indicator style
    lv_style_init(&style_indicator);
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, theme->primaryColor);
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);

    // knob style
    lv_style_init(&style_knob);
    lv_style_set_bg_opa(&style_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&style_knob, theme->primaryColor);
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
    lv_style_set_border_color(&style_knob, theme->backgroundColor);
    lv_style_set_border_width(&style_knob, 2);
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&style_knob, 6);

    // Create slider
    lv_obj_t* screenBrightnessSlider = lv_slider_create(screenSettingsContainer);
    lv_obj_remove_style_all(screenBrightnessSlider);
    lv_obj_set_size(screenBrightnessSlider, 264, 32);
    lv_obj_align(screenBrightnessSlider, LV_ALIGN_TOP_LEFT, 16, 32);
    lv_obj_add_style(screenBrightnessSlider, &style_main, LV_PART_MAIN);
    lv_obj_add_style(screenBrightnessSlider, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(screenBrightnessSlider, &style_knob, LV_PART_KNOB);

    // screen brightness label

    if (backlightPWM) {
        lv_obj_t* screenBrightnessLabel = lv_label_create(screenSettingsContainer);
        lv_label_set_text(screenBrightnessLabel, "SCREEN BRIGHTNESS");
        lv_obj_set_style_text_font(screenBrightnessLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(screenBrightnessLabel, theme->textColor, LV_PART_MAIN);
        lv_obj_align_to(screenBrightnessLabel, screenBrightnessSlider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        lv_slider_set_value(screenBrightnessSlider, 100, LV_ANIM_OFF);
    } else {
        lv_obj_t* screenBrightnessLabel = lv_label_create(screenSettingsContainer);
        lv_label_set_text(screenBrightnessLabel, "SCREEN BRIGHTNESS NOT AVAILABLE");
        lv_obj_set_style_text_font(screenBrightnessLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(screenBrightnessLabel, theme->textColor, LV_PART_MAIN);
        lv_obj_align_to(screenBrightnessLabel, screenBrightnessSlider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        lv_slider_set_value(screenBrightnessSlider, 0, LV_ANIM_OFF);
    }


    // Asic Settings Container
    lv_obj_t* asicSettingsContainer = lv_obj_create(asicSettingsTab);
    lv_obj_set_size(asicSettingsContainer, 672, 312);
    lv_obj_align(asicSettingsContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(asicSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(asicSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(asicSettingsContainer, LV_OBJ_FLAG_SCROLLABLE);

/*
    // Asic Settings Label
    lv_obj_t* asicSettingsLabel = lv_label_create(asicSettingsContainer);
    lv_label_set_text(asicSettingsLabel, "ASIC SETTINGS");
    lv_obj_set_style_text_font(asicSettingsLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(asicSettingsLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(asicSettingsLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(asicSettingsLabel, LV_OBJ_FLAG_SCROLLABLE);
*/
    /*
    //Asic Freqyency Text Area Label
    lv_obj_t* asicFrequencyLabel = lv_label_create(asicSettingsContainer);
    lv_label_set_text(asicFrequencyLabel, "Range: 450 - 575MHz");
    lv_obj_set_style_text_font(asicFrequencyLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(asicFrequencyLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(asicFrequencyLabel, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_clear_flag(asicFrequencyLabel, LV_OBJ_FLAG_SCROLLABLE);
    
    // Asic Frequency text area
    // TODO Add Flag to change dropdown options for each model
    lv_obj_t* asicFrequencyTextArea = setTextAreaStyles(asicSettingsContainer, "Asic Frequency");
    lv_obj_align(asicFrequencyTextArea, LV_ALIGN_TOP_LEFT, 0, 40);
    lv_obj_set_width(asicFrequencyTextArea, lv_pct(45));
    lv_obj_add_event_cb(asicFrequencyTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(asicFrequencyTextArea, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(asicFrequencyTextArea);

    // Asic Voltage Text Area Label
    lv_obj_t* asicVoltageLabel = lv_label_create(asicSettingsContainer);
    lv_label_set_text(asicVoltageLabel, "Range: 1000 - 1250 mV");
    lv_obj_set_style_text_font(asicVoltageLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(asicVoltageLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(asicVoltageLabel, LV_ALIGN_TOP_LEFT, 0, 104);
    lv_obj_clear_flag(asicVoltageLabel, LV_OBJ_FLAG_SCROLLABLE);
    
    //Asic Voltage text area
    lv_obj_t* asicVoltageTextArea = setTextAreaStyles(asicSettingsContainer, "Asic Voltage");
    lv_obj_align(asicVoltageTextArea, LV_ALIGN_TOP_LEFT, 0, 128);
    lv_obj_set_width(asicVoltageTextArea, lv_pct(45));
    lv_obj_add_event_cb(asicVoltageTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(asicVoltageTextArea, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(asicVoltageTextArea);
    */

// Power Mode Container
lv_obj_t* powerModeContainer = lv_obj_create(asicSettingsContainer);
lv_obj_set_size(powerModeContainer, 672, 320);
lv_obj_align(powerModeContainer, LV_ALIGN_TOP_LEFT, 0, -16);  
lv_obj_set_style_bg_opa(powerModeContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(powerModeContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(powerModeContainer, LV_OBJ_FLAG_SCROLLABLE);

// Power Mode Label
lv_obj_t* powerModeLabel = lv_label_create(powerModeContainer);
lv_label_set_text(powerModeLabel, "POWER MODE");
lv_obj_set_style_text_font(powerModeLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(powerModeLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(powerModeLabel, LV_ALIGN_TOP_LEFT, 0, 0);  

// Create radio button style
static lv_style_t style_radio;
static lv_style_t style_radio_chk;  // Add new style for checked state

lv_style_init(&style_radio);
lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
lv_style_set_bg_color(&style_radio, theme->backgroundColor);
lv_style_set_bg_opa(&style_radio, LV_OPA_100); 
lv_style_set_border_color(&style_radio, theme->borderColor);
lv_style_set_border_width(&style_radio, 2);
lv_style_set_border_opa(&style_radio, LV_OPA_100);
lv_style_set_text_color(&style_radio, theme->textColor);
lv_style_set_text_font(&style_radio, theme->fontMedium16);

// Initialize checked state style
lv_style_init(&style_radio_chk);
lv_style_set_bg_color(&style_radio_chk, theme->primaryColor);
lv_style_set_bg_opa(&style_radio_chk, LV_OPA_100);

// Low Power Mode Checkbox
lowPowerMode = lv_checkbox_create(powerModeContainer);
lv_checkbox_set_text(lowPowerMode, "LOW POWER MODE | Quiet Fans");
lv_obj_add_style(lowPowerMode, &style_radio, LV_PART_INDICATOR);
lv_obj_add_style(lowPowerMode, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
lv_obj_align(lowPowerMode, LV_ALIGN_TOP_LEFT, 0, 40);  
lv_obj_set_style_text_font(lowPowerMode, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(lowPowerMode, theme->textColor, LV_PART_MAIN);
lv_obj_add_event_cb(lowPowerMode, power_mode_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

// Normal Power Mode Checkbox
normalPowerMode = lv_checkbox_create(powerModeContainer);
lv_checkbox_set_text(normalPowerMode, "NORMAL POWER MODE | Balanced performance");
lv_obj_add_style(normalPowerMode, &style_radio, LV_PART_INDICATOR);
lv_obj_add_style(normalPowerMode, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
lv_obj_align(normalPowerMode, LV_ALIGN_TOP_LEFT, 0, 120); 
lv_obj_set_style_text_font(normalPowerMode, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(normalPowerMode, theme->textColor, LV_PART_MAIN);
lv_obj_add_event_cb(normalPowerMode, power_mode_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

// High Power Mode Checkbox
highPowerMode = lv_checkbox_create(powerModeContainer);
lv_checkbox_set_text(highPowerMode, "HIGH POWER MODE | Increased Fan Noise");
lv_obj_add_style(highPowerMode, &style_radio, LV_PART_INDICATOR);
lv_obj_add_style(highPowerMode, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
lv_obj_align(highPowerMode, LV_ALIGN_TOP_LEFT, 0, 200);  // Moved up by 32 (from 200 to 168)
lv_obj_set_style_text_font(highPowerMode, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(highPowerMode, theme->textColor, LV_PART_MAIN);
lv_obj_add_event_cb(highPowerMode, power_mode_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

// Set default selection null
//lv_obj_add_state(normalPowerMode, LV_STATE_CHECKED);

// Group radio buttons together
lv_group_t* radio_group = lv_group_create();
lv_group_add_obj(radio_group, lowPowerMode);
lv_group_add_obj(radio_group, normalPowerMode);
lv_group_add_obj(radio_group, highPowerMode);

// Autotune Settings Container
lv_obj_t* autoTuneSettingsContainer = lv_obj_create(autoTuneSettingsTab);
lv_obj_set_size(autoTuneSettingsContainer, 672, 312);
lv_obj_align(autoTuneSettingsContainer, LV_ALIGN_CENTER, 0, 0);
lv_obj_set_style_bg_opa(autoTuneSettingsContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(autoTuneSettingsContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(autoTuneSettingsContainer, LV_OBJ_FLAG_SCROLLABLE);


// Target frequency, voltage, and fan speed
lv_obj_t* targetVoltageContainer = lv_obj_create(autoTuneSettingsContainer);
lv_obj_set_size(targetVoltageContainer, 208, 136);
lv_obj_align(targetVoltageContainer, LV_ALIGN_TOP_LEFT, 0, 0);
lv_obj_set_style_bg_opa(targetVoltageContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(targetVoltageContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(targetVoltageContainer, LV_OBJ_FLAG_SCROLLABLE);

//targetVoltageLabel
lv_obj_t* targetVoltageLabel = lv_label_create(targetVoltageContainer);
lv_label_set_text(targetVoltageLabel, "TARGET VOLT");
lv_obj_set_style_text_font(targetVoltageLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(targetVoltageLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(targetVoltageLabel, LV_ALIGN_TOP_MID, 0, -16); 

//targetVoltageVariableLabel
targetVoltageVariableLabel = lv_label_create(targetVoltageContainer);
lv_label_set_text(targetVoltageVariableLabel, "1250mV");
lv_obj_set_style_text_font(targetVoltageVariableLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(targetVoltageVariableLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(targetVoltageVariableLabel, LV_ALIGN_TOP_MID, 0, 16); 

lv_obj_t* targetFrequencyContainer = lv_obj_create(autoTuneSettingsContainer);
lv_obj_set_size(targetFrequencyContainer, 208, 136);
lv_obj_align(targetFrequencyContainer, LV_ALIGN_TOP_LEFT, 210, 0);
lv_obj_set_style_bg_opa(targetFrequencyContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(targetFrequencyContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(targetFrequencyContainer, LV_OBJ_FLAG_SCROLLABLE);

//targetFrequencyLabel
lv_obj_t* targetFrequencyLabel = lv_label_create(targetFrequencyContainer);
lv_label_set_text(targetFrequencyLabel, "TARGET FREQ");
lv_obj_set_style_text_font(targetFrequencyLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(targetFrequencyLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(targetFrequencyLabel, LV_ALIGN_TOP_MID, 0, -16); 

//targetFrequencyVariableLabel
targetFrequencyVariableLabel = lv_label_create(targetFrequencyContainer);
lv_label_set_text(targetFrequencyVariableLabel, "490 MHz");
lv_obj_set_style_text_font(targetFrequencyVariableLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(targetFrequencyVariableLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(targetFrequencyVariableLabel, LV_ALIGN_TOP_MID, 0, 16); 

lv_obj_t* targetFanSpeedContainer = lv_obj_create(autoTuneSettingsContainer);
lv_obj_set_size(targetFanSpeedContainer, 208, 136);
lv_obj_align(targetFanSpeedContainer, LV_ALIGN_TOP_LEFT, 420, 0);
lv_obj_set_style_bg_opa(targetFanSpeedContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(targetFanSpeedContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(targetFanSpeedContainer, LV_OBJ_FLAG_SCROLLABLE);

//targetFanSpeedLabel
lv_obj_t* targetFanSpeedLabel = lv_label_create(targetFanSpeedContainer);
lv_label_set_text(targetFanSpeedLabel, "TARGET FAN");
lv_obj_set_style_text_font(targetFanSpeedLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(targetFanSpeedLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(targetFanSpeedLabel, LV_ALIGN_TOP_MID, 0, -16); 

//targetFanSpeedVariableLabel
targetFanSpeedVariableLabel = lv_label_create(targetFanSpeedContainer);
lv_label_set_text(targetFanSpeedVariableLabel, "35%");
lv_obj_set_style_text_font(targetFanSpeedVariableLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(targetFanSpeedVariableLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(targetFanSpeedVariableLabel, LV_ALIGN_TOP_MID, 0, 16); 


// current offset
lv_obj_t* offestVoltageContainer = lv_obj_create(autoTuneSettingsContainer);
lv_obj_set_size(offestVoltageContainer, 208, 136);
lv_obj_align(offestVoltageContainer, LV_ALIGN_TOP_LEFT, 0, 136);
lv_obj_set_style_bg_opa(offestVoltageContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(offestVoltageContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(offestVoltageContainer, LV_OBJ_FLAG_SCROLLABLE);

//offsetVoltageLabel
lv_obj_t* offsetVoltageLabel = lv_label_create(offestVoltageContainer);
lv_label_set_text(offsetVoltageLabel, "OFFSET VOLT");
lv_obj_set_style_text_font(offsetVoltageLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(offsetVoltageLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(offsetVoltageLabel, LV_ALIGN_TOP_MID, 0, -16); 

//offsetVoltageVariableLabel
offsetVoltageVariableLabel = lv_label_create(offestVoltageContainer);
lv_label_set_text(offsetVoltageVariableLabel, "-120mV");
lv_obj_set_style_text_font(offsetVoltageVariableLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(offsetVoltageVariableLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(offsetVoltageVariableLabel, LV_ALIGN_TOP_MID, 0, 16); 

lv_obj_t* offsetFrequencyContainer = lv_obj_create(autoTuneSettingsContainer);
lv_obj_set_size(offsetFrequencyContainer, 208, 136);
lv_obj_align(offsetFrequencyContainer, LV_ALIGN_TOP_LEFT, 210, 136);
lv_obj_set_style_bg_opa(offsetFrequencyContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(offsetFrequencyContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(offsetFrequencyContainer, LV_OBJ_FLAG_SCROLLABLE);

//offsetFrequencyLabel
lv_obj_t* offsetFrequencyLabel = lv_label_create(offsetFrequencyContainer);
lv_label_set_text(offsetFrequencyLabel, "OFFSET FREQ");
lv_obj_set_style_text_font(offsetFrequencyLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(offsetFrequencyLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(offsetFrequencyLabel, LV_ALIGN_TOP_MID, 0, -16); 

//offsetFrequencyVariableLabel
offsetFrequencyVariableLabel = lv_label_create(offsetFrequencyContainer);
lv_label_set_text(offsetFrequencyVariableLabel, "-20 MHz");
lv_obj_set_style_text_font(offsetFrequencyVariableLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(offsetFrequencyVariableLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(offsetFrequencyVariableLabel, LV_ALIGN_TOP_MID, 0, 16); 

lv_obj_t* offsetFanSpeedContainer = lv_obj_create(autoTuneSettingsContainer);
lv_obj_set_size(offsetFanSpeedContainer, 208, 136);
lv_obj_align(offsetFanSpeedContainer, LV_ALIGN_TOP_LEFT, 420, 136);
lv_obj_set_style_bg_opa(offsetFanSpeedContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_set_style_border_opa(offsetFanSpeedContainer, LV_OPA_0, LV_PART_MAIN);
lv_obj_clear_flag(offsetFanSpeedContainer, LV_OBJ_FLAG_SCROLLABLE);

//offsetFanSpeedLabel
lv_obj_t* offsetFanSpeedLabel = lv_label_create(offsetFanSpeedContainer);
lv_label_set_text(offsetFanSpeedLabel, "OFFSET FAN");
lv_obj_set_style_text_font(offsetFanSpeedLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(offsetFanSpeedLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(offsetFanSpeedLabel, LV_ALIGN_TOP_MID, 0, -16); 

//offsetFanSpeedVariableLabel
offsetFanSpeedVariableLabel = lv_label_create(offsetFanSpeedContainer);
lv_label_set_text(offsetFanSpeedVariableLabel, "3%");
lv_obj_set_style_text_font(offsetFanSpeedVariableLabel, theme->fontMedium24, LV_PART_MAIN);
lv_obj_set_style_text_color(offsetFanSpeedVariableLabel, theme->textColor, LV_PART_MAIN);
lv_obj_align(offsetFanSpeedVariableLabel, LV_ALIGN_TOP_MID, 0, 16); 

// Create switch
lv_obj_t* autotuneSwitch = lv_switch_create(autoTuneSettingsContainer);
lv_obj_set_size(autotuneSwitch, 80, 40);  // Set width=60px, height=32px
lv_obj_align(autotuneSwitch, LV_ALIGN_BOTTOM_RIGHT, -16, 16);

// Create label for the switch
autotuneSwitchLabel = lv_label_create(autoTuneSettingsContainer);
lv_obj_align_to(autotuneSwitchLabel, autotuneSwitch, LV_ALIGN_OUT_LEFT_MID, -64, -17);

// Apply theme styling to label
lv_obj_set_style_text_color(autotuneSwitchLabel, theme->textColor, LV_PART_MAIN);
lv_obj_set_style_text_font(autotuneSwitchLabel, theme->fontMedium16, LV_PART_MAIN);

// Apply theme styling to switch
lv_obj_set_style_bg_color(autotuneSwitch, theme->primaryColor, LV_PART_INDICATOR | LV_STATE_CHECKED);
lv_obj_set_style_bg_color(autotuneSwitch, theme->backgroundColor, LV_PART_MAIN);
lv_obj_set_style_border_color(autotuneSwitch, theme->borderColor, LV_PART_MAIN);
lv_obj_set_style_border_width(autotuneSwitch, 2, LV_PART_MAIN);

// Set initial switch state based on autoTuneEnabled flag
lv_obj_add_state(autotuneSwitch, autoTuneEnabled ? LV_STATE_CHECKED : 0);

// Set label text based on autoTuneEnabled flag
if(autoTuneEnabled) {
    lv_label_set_text(autotuneSwitchLabel, "Autotune\nEnabled");
} else {
    lv_label_set_text(autotuneSwitchLabel, "Autotune\nDisabled");
}

lv_obj_add_event_cb(autotuneSwitch, autotuneSwitchEventHandler, LV_EVENT_ALL, NULL);
lv_obj_add_flag(autotuneSwitch, LV_OBJ_FLAG_EVENT_BUBBLE);


static lv_obj_t* autoTuneLabels[6];  // Array to hold all autotune labels
autoTuneLabels[0] = targetVoltageVariableLabel;
autoTuneLabels[1] = targetFrequencyVariableLabel;
autoTuneLabels[2] = targetFanSpeedVariableLabel;
autoTuneLabels[3] = offsetVoltageVariableLabel;
autoTuneLabels[4] = offsetFrequencyVariableLabel;
autoTuneLabels[5] = offsetFanSpeedVariableLabel;

// Create timer for autotune settings updates
screenObjs.autoTuneSettingsTimer = lv_timer_create(updateAutoTuneLabels, 1000, autoTuneLabels);


    // Theme Settings Container
    lv_obj_t* themeSettingsContainer = lv_obj_create(themeSettingsTab);
    lv_obj_set_size(themeSettingsContainer, 672, 312);
    lv_obj_align(themeSettingsContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(themeSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(themeSettingsContainer, LV_OPA_0, LV_PART_MAIN);



    // Theme Dropdown
    themeDropdown = lv_dropdown_create(themeSettingsContainer);

        // style list
    lv_obj_t* themeDropdownList = lv_dropdown_get_list(themeDropdown);
   
    lv_obj_set_style_bg_color(themeDropdownList, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_border_color(themeDropdownList, theme->borderColor, LV_PART_MAIN );
    lv_obj_set_style_text_color(themeDropdownList, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_font(themeDropdownList, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(themeDropdownList, theme->primaryColor, LV_PART_SELECTED);
    lv_obj_set_style_bg_color(themeDropdownList, theme->primaryColor, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(themeDropdownList, theme->primaryColor, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(themeDropdownList, theme->backgroundColor, LV_PART_SELECTED | LV_STATE_CHECKED);

    //lv_obj_remove_style_all(modelDropdown);
    lv_obj_set_size(themeDropdown, 200, 48);
    lv_obj_align(themeDropdown, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_set_width(themeDropdown, lv_pct(35));
    lv_dropdown_set_dir(themeDropdown, LV_DIR_BOTTOM);
    lv_obj_set_style_text_align(themeDropdown, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED);
    
    lv_obj_set_style_bg_color(themeDropdown, theme->backgroundColor, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(themeDropdown, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_text_color(themeDropdown, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_font(themeDropdown, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_font(themeDropdown, LV_FONT_DEFAULT, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(themeDropdown, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(themeDropdown, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(themeDropdown, theme->primaryColor, LV_PART_SELECTED);
    lv_obj_set_style_text_color(themeDropdown, theme->backgroundColor, LV_PART_SELECTED);

    lv_dropdown_set_options(themeDropdown, "ACS DEFAULT\nBITAXE RED");
    #if (BlockStreamJade == 1)
    lv_dropdown_add_option(themeDropdown, "BLOCKSTREAM JADE\nBLOCKSTREAM BLUE", LV_DROPDOWN_POS_LAST);
    #endif
    #if (SoloSatoshi == 1)
    lv_dropdown_add_option(themeDropdown, "SOLO SATOSHI", LV_DROPDOWN_POS_LAST);
    #endif
    #if (ALTAIR == 1)
    lv_dropdown_add_option(themeDropdown, "ALTAIR", LV_DROPDOWN_POS_LAST);
    #endif
    #if (SoloMiningCo == 1)
    lv_dropdown_add_option(themeDropdown, "SOLO MINING CO", LV_DROPDOWN_POS_LAST);
    #endif
    #if (BTCMagazine == 1)
    lv_dropdown_add_option(themeDropdown, "BTCMAGAZINE", LV_DROPDOWN_POS_LAST);
    #endif
    #if (VoskCoin == 1)
    lv_dropdown_add_option(themeDropdown, "VOSKCOIN", LV_DROPDOWN_POS_LAST);
    #endif
    // Add spacing at the end of the list to make last option selectable
    lv_dropdown_add_option(themeDropdown, "", LV_DROPDOWN_POS_LAST);


    lv_obj_add_event_cb(themeDropdown, themeDropdownEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    //UI Theme Example Container    
    themeExampleContainer = lv_obj_create(themeSettingsContainer);
    lv_obj_set_size(themeExampleContainer, 400, 240);
    lv_obj_align(themeExampleContainer, LV_ALIGN_BOTTOM_RIGHT, 0, -8);
    lv_obj_set_style_bg_opa(themeExampleContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(themeExampleContainer, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_radius(themeExampleContainer, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(themeExampleContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(themeExampleContainer, theme->borderColor, LV_PART_MAIN);
    lv_obj_clear_flag(themeExampleContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Theme Preview Label
    themePreviewLabel = lv_label_create(themeSettingsContainer);
    lv_label_set_text(themePreviewLabel, "Theme Preview");
    lv_obj_set_style_text_font(themePreviewLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(themePreviewLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(themePreviewLabel, LV_ALIGN_TOP_RIGHT, 0, -16);
    lv_obj_clear_flag(themePreviewLabel, LV_OBJ_FLAG_SCROLLABLE);
    

    // Theme Example Image
    themeExampleImage = lv_img_create(themeExampleContainer);
    lv_img_set_src(themeExampleImage, theme->themePreview);
    Serial0.println(theme->themePreview);
    lv_obj_align(themeExampleImage, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(themeExampleImage, 16, LV_PART_MAIN);

    // Time Settings Container
    timeSettingsContainer = lv_obj_create(timeSettingsTab);
    lv_obj_set_size(timeSettingsContainer, 672, 320);
    lv_obj_align(timeSettingsContainer, LV_ALIGN_TOP_LEFT, 0, -16);  
    lv_obj_set_style_bg_opa(timeSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(timeSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(timeSettingsContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Time Zone Label
    lv_obj_t* timeZoneLabel = lv_label_create(timeSettingsContainer);
    lv_label_set_text(timeZoneLabel, "Time Zone");
    lv_obj_set_style_text_font(timeZoneLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(timeZoneLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(timeZoneLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(timeZoneLabel, LV_OBJ_FLAG_SCROLLABLE);

    // Create Time Zone List
    timeZoneList = lv_list_create(timeSettingsContainer);
    lv_obj_set_size(timeZoneList, 280, 264);
    lv_obj_align(timeZoneList, LV_ALIGN_LEFT_MID, 0, 16);
    lv_obj_set_style_bg_color(timeZoneList, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(timeZoneList, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(timeZoneList, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(timeZoneList, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_opa(timeZoneList, LV_OPA_100, LV_PART_MAIN);

    // Add time zones to the list
    const char* timeZones[] = {
        "Custom Offset", "EST -5", "CST -6", "MST -7", "PST -8", 
        "AKST -9", "HST -10", "GMT +0", "CET +1", "AWST +8",
        "ACST +9:30", "AEST +10", "ACDT +10:30", "AEDT +11"
    };

    for (const char* tz : timeZones) {
        lv_obj_t* btn = lv_list_add_btn(timeZoneList, "S:/clock40x40.png", tz);
        
        // Style the button
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_STATE_PRESSED);
        lv_obj_set_style_text_color(btn, theme->textColor, LV_PART_MAIN);
        lv_obj_set_style_text_font(btn, theme->fontMedium24, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, timeZoneListEventHandler, LV_EVENT_CLICKED, NULL);

        // Style the clock icon
        lv_obj_t* img = lv_obj_get_child(btn, 0);
        lv_obj_set_style_img_recolor(img, theme->primaryColor, LV_PART_MAIN);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    }
    #if (DEBUG_UI == 1)
    // Debug setting container 
    debugSettingsContainer = lv_obj_create(debugSettingsTab);
    lv_obj_set_size(debugSettingsContainer, 672, 312);
    lv_obj_align(debugSettingsContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(debugSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(debugSettingsContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(debugSettingsContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Debug Settings Label
    lv_obj_t* debugSettingsLabel = lv_label_create(debugSettingsContainer);
    lv_label_set_text(debugSettingsLabel, "Debug Settings");
    lv_obj_set_style_text_font(debugSettingsLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(debugSettingsLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(debugSettingsLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(debugSettingsLabel, LV_OBJ_FLAG_SCROLLABLE);
    

    // Toggle Overheat mode button
    lv_obj_t* overheatModeButton = lv_btn_create(debugSettingsContainer);
    lv_obj_set_size(overheatModeButton, 240, 96);
    lv_obj_align(overheatModeButton, LV_ALIGN_TOP_RIGHT, -16, 16);
    lv_obj_set_style_bg_color(overheatModeButton, lv_color_hex(0xff0000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overheatModeButton, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(overheatModeButton, overheatModeButtonEventHandler, LV_EVENT_CLICKED, NULL);

    // Toggle Overheat mode label
    lv_obj_t* overheatModeLabel = lv_label_create(overheatModeButton);
    lv_label_set_text(overheatModeLabel, "Overheat Mode");
    lv_obj_set_style_text_font(overheatModeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(overheatModeLabel, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(overheatModeLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(overheatModeLabel, LV_OBJ_FLAG_SCROLLABLE);

    // Toggle block found button
    lv_obj_t* blockFoundButton = lv_btn_create(debugSettingsContainer);
    lv_obj_set_size(blockFoundButton, 240, 96);
    lv_obj_align(blockFoundButton, LV_ALIGN_TOP_RIGHT, -16, 144);
    lv_obj_set_style_bg_color(blockFoundButton, lv_color_hex(0xff0000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(blockFoundButton, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(blockFoundButton, blockFoundButtonEventHandler, LV_EVENT_CLICKED, NULL);

    // Toggle Block Found mode label
    lv_obj_t* blockFoundLabel = lv_label_create(blockFoundButton);
    lv_label_set_text(blockFoundLabel, "Block Found");
    lv_obj_set_style_text_font(blockFoundLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(blockFoundLabel, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(blockFoundLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(blockFoundLabel, LV_OBJ_FLAG_SCROLLABLE);
    #endif


    // Save Tab Container
    lv_obj_t* saveTabContainer = lv_obj_create(saveSettingsTab);
    lv_obj_set_size(saveTabContainer, 672, 312);
    lv_obj_align(saveTabContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(saveTabContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(saveTabContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(saveTabContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Save Tab Label
    lv_obj_t* saveTabLabel = lv_label_create(saveTabContainer);
    lv_label_set_text(saveTabLabel, "Double Check Settings on all tabs\nPress SAVE to apply and Restart");
    lv_obj_set_style_text_font(saveTabLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(saveTabLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(saveTabLabel, LV_ALIGN_TOP_LEFT, 0, -16);
    lv_obj_clear_flag(saveTabLabel, LV_OBJ_FLAG_SCROLLABLE);


    // OTA QR Code Container
    lv_obj_t* otaQRCodeContainer = lv_obj_create(saveTabContainer);
    lv_obj_set_size(otaQRCodeContainer, 200, 232);
    lv_obj_align(otaQRCodeContainer, LV_ALIGN_BOTTOM_LEFT, 0, -8);
    lv_obj_set_style_bg_opa(otaQRCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(otaQRCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(otaQRCodeContainer, LV_OBJ_FLAG_SCROLLABLE);
    // URL String
    char urlString[100];
    String ipString = WiFi.localIP().toString();
    snprintf(urlString, sizeof(urlString), "http://%s/ota", ipString.c_str());

    // OTA QR Code
    lv_obj_t* otaQRCode = lv_qrcode_create(otaQRCodeContainer, 128, theme->primaryColor, theme->backgroundColor);
    if (WiFi.status() == WL_CONNECTED) 
    {
        lv_qrcode_update(otaQRCode, urlString, strlen(urlString));
    }
    lv_obj_align(otaQRCode, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // OTA Address Label
    char otaAddressLabelText[100];
    snprintf(otaAddressLabelText, sizeof(otaAddressLabelText), "Firmware Update:\n%s/ota", ipString.c_str());
    lv_obj_t* otaAddressLabel = lv_label_create(otaQRCodeContainer);
    if (WiFi.status() == WL_CONNECTED) {
        lv_label_set_text(otaAddressLabel, otaAddressLabelText);
    }
    else {
        lv_label_set_text(otaAddressLabel, "Firmware Update:\nConnect to WiFi");
    }
    lv_obj_set_style_text_font(otaAddressLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(otaAddressLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(otaAddressLabel, LV_ALIGN_TOP_LEFT, 0, 8);
    lv_obj_clear_flag(otaAddressLabel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_align(otaAddressLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

    // Version Label
    char versionLabelText[100];
    snprintf(versionLabelText, sizeof(versionLabelText), "Version: %d.%d.%d %s %s\nVersion Date: %s", MajorVersion, MinorVersion, PatchVersion, VersionNote, ModelType, BuildDate);
    lv_obj_t* versionLabel = lv_label_create(saveTabContainer);
    lv_label_set_text(versionLabel, versionLabelText);
    lv_obj_set_style_text_font(versionLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(versionLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(versionLabel, LV_ALIGN_BOTTOM_LEFT, 0, 24);
    lv_obj_clear_flag(versionLabel, LV_OBJ_FLAG_SCROLLABLE);

    // Factory Reset Button
    lv_obj_t* factoryResetButton = lv_btn_create(saveTabContainer);
    lv_obj_set_size(factoryResetButton, 240, 96);
    lv_obj_align(factoryResetButton, LV_ALIGN_TOP_RIGHT, -16, 24);

    // Style the button and label
    lv_obj_set_style_bg_color(factoryResetButton, lv_color_hex(0xff0000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(factoryResetButton, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_text_font(factoryResetButton, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(factoryResetButton, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_t* factoryResetLabel = lv_label_create(factoryResetButton);
    lv_label_set_text(factoryResetLabel, "Factory Reset");
    lv_obj_center(factoryResetLabel);
    lv_obj_add_event_cb(factoryResetButton, factoryResetButtonEventHandler, LV_EVENT_CLICKED, NULL);

    // Save Button
    lv_obj_t* saveButton = lv_btn_create(saveTabContainer);
    lv_obj_set_size(saveButton, 240, 96);
    lv_obj_align(saveButton, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
        
        
        // Create label for save button
        lv_obj_t* saveLabel = lv_label_create(saveButton);
        lv_label_set_text(saveLabel, "SAVE");
        lv_obj_center(saveLabel);
        
        // Style the button and label
        lv_obj_set_style_bg_color(saveButton, theme->primaryColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(saveButton, LV_OPA_60, LV_PART_MAIN);
        lv_obj_set_style_text_font(saveLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(saveLabel, theme->textColor, LV_PART_MAIN);

        // Add event handler
    if (saveButton != NULL) {
        lv_obj_add_event_cb(saveButton, saveButtonEventHandler, LV_EVENT_CLICKED, NULL);
    }

    // Keyboard Creation
        kb = lv_keyboard_create(lv_scr_act());
        lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
        // Remove all default styles
        lv_obj_remove_style_all(kb);
        // Set keyboard background
        lv_obj_set_style_bg_color(kb, theme->backgroundColor, LV_PART_MAIN);  // Dark background
        lv_obj_set_style_bg_opa(kb, LV_OPA_80, LV_PART_MAIN);
        // Style for buttons (single state only)
        lv_obj_set_style_bg_color(kb, theme->backgroundColor, LV_PART_ITEMS);  // Dark button background
        lv_obj_set_style_text_color(kb, theme->textColor, LV_PART_ITEMS);  // Your theme color for text
        lv_obj_set_style_border_width(kb, 2, LV_PART_ITEMS);
        lv_obj_set_style_border_color(kb, theme->borderColor, LV_PART_ITEMS);
        lv_obj_set_style_radius(kb, 24, LV_PART_ITEMS);
        // Regular keys
        lv_obj_set_style_text_font(kb, theme->fontMedium16, LV_PART_ITEMS);
        // Symbols that Inter Font doesn't support
        lv_obj_set_style_text_font(kb, LV_FONT_DEFAULT, LV_PART_ITEMS | LV_STATE_CHECKED);
        // Disable ALL animations and transitions
        lv_obj_set_style_anim_time(kb, 0, 0);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_SCROLL_CHAIN);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
        //lv_keyboard_set_textarea(kb, stratumUrlTextAreaMain);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

        // Create save button
        


}



void showSettingsConfirmationOverlay() {
    static lv_obj_t* overlay = NULL;
    uiTheme_t* theme = getCurrentTheme();
    if (settingsChanged && !overlay) {
        Serial.println("Creating overlay");
        // Create semi-transparent overlay
        overlay = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(overlay);
        lv_obj_set_size(overlay, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(overlay, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(overlay, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_radius(overlay, 0, LV_PART_MAIN);
        
        // Create confirmation container
        lv_obj_t* confirmContainer = lv_obj_create(overlay);
        lv_obj_set_size(confirmContainer, 400, 200);
        lv_obj_center(confirmContainer);
        lv_obj_set_style_bg_color(confirmContainer, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(confirmContainer, theme->borderColor, LV_PART_MAIN);
        lv_obj_set_style_border_width(confirmContainer, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(confirmContainer, 16, LV_PART_MAIN);
        
        // Add message
        lv_obj_t* message = lv_label_create(confirmContainer);
        lv_label_set_text(message, "Waiting to send settings...");
        lv_obj_set_style_text_font(message, theme->fontMedium24, LV_PART_MAIN);
        lv_obj_set_style_text_color(message, theme->textColor, LV_PART_MAIN);
        lv_obj_align(message, LV_ALIGN_CENTER, 0, 0);
    } else if (!settingsChanged && overlay) {
        Serial.println("Removing overlay");
        // Remove overlay when settings are no longer being changed
        lv_obj_del(overlay);
        overlay = NULL;
    }
}

static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(kb != NULL) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            // Disable animation for showing keyboard
            lv_obj_set_style_anim_time(kb, 0, 0);

            // kyboard mode number
            if (ta == settingsTextAreas.stratumPortTextArea || ta == settingsTextAreas.stratumPortTextAreaFallback ||
                ta == settingsTextAreas.asicVoltageTextArea || ta == settingsTextAreas.asicFrequencyTextArea) 
            {
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
                lv_obj_set_style_text_font(kb, LV_FONT_DEFAULT, LV_PART_ITEMS); 
            }
            else if (ta == customOffsetHoursTextArea || ta == customOffsetMinutesTextArea)
            {
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
                lv_obj_set_style_text_font(kb, LV_FONT_DEFAULT, LV_PART_ITEMS); 
            }
            else
            {
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
        }
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED) {
        LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
        // Disable animation for hiding keyboard
        lv_obj_set_style_anim_time(kb, 0, 0);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

void showOverheatOverlay()
{
    uiTheme_t* theme = getCurrentTheme();
    static lv_obj_t* overheatOverlay = NULL;
    
    if (confirmedOverheatMode && !overheatOverlay) {
        lvgl_port_lock(-1);
        Serial.println("Creating overlay");
        // Create semi-transparent overlay
        overheatOverlay = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(overheatOverlay);
        lv_obj_set_size(overheatOverlay, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(overheatOverlay, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(overheatOverlay, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_radius(overheatOverlay, 0, LV_PART_MAIN);
        lv_obj_clear_flag(overheatOverlay, LV_OBJ_FLAG_SCROLLABLE);
        
        // Create confirmation container
        lv_obj_t* confirmContainer = lv_obj_create(overheatOverlay);
        lv_obj_set_size(confirmContainer, 672, 392);
        lv_obj_align(confirmContainer, LV_ALIGN_CENTER, 40, 28);
        lv_obj_set_style_bg_color(confirmContainer, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(confirmContainer, theme->borderColor, LV_PART_MAIN);
        lv_obj_set_style_border_width(confirmContainer, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(confirmContainer, 16, LV_PART_MAIN);
        lv_obj_clear_flag(confirmContainer, LV_OBJ_FLAG_SCROLLABLE);
        
        // Add message
        lv_obj_t* message = lv_label_create(confirmContainer);
        lv_label_set_text(message, "OVERHEAT MODE\nRESET TO\nSAFE SETTINGS");
        lv_obj_set_style_text_font(message, theme->fontExtraBold56, LV_PART_MAIN);
        lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(message, theme->textColor, LV_PART_MAIN);
        lv_obj_align(message, LV_ALIGN_TOP_MID, 0, 16);

         // not needed when mode is already known
         #if (BitaxeUltra == 0 && BitaxeSupra == 0 && BitaxeGamma == 0)
        lv_obj_t* modelDropdown = lv_dropdown_create(confirmContainer);
        //lv_obj_remove_style_all(modelDropdown);
        lv_obj_set_size(modelDropdown, 200, 48);

        lv_obj_align(modelDropdown, LV_ALIGN_CENTER, 0, 48);
        lv_obj_set_style_text_align(modelDropdown, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED);
        
        lv_obj_set_style_bg_color(modelDropdown, theme->backgroundColor, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(modelDropdown, theme->borderColor, LV_PART_MAIN);
        lv_obj_set_style_text_color(modelDropdown, theme->textColor, LV_PART_MAIN);
        lv_obj_set_style_text_font(modelDropdown, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_font(modelDropdown, LV_FONT_DEFAULT, LV_PART_INDICATOR);
        lv_obj_set_style_border_width(modelDropdown, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(modelDropdown, 16, LV_PART_MAIN);
        lv_obj_set_style_bg_color(modelDropdown, theme->primaryColor, LV_PART_SELECTED);
        lv_obj_set_style_text_color(modelDropdown, theme->backgroundColor, LV_PART_SELECTED);

        lv_dropdown_set_options(modelDropdown, "Bitaxe Ultra\nBitaxe Supra\nBitaxe Gama");

        // style list
        lv_obj_t* modelDropdownList = lv_dropdown_get_list(modelDropdown);
        lv_obj_set_style_bg_color(modelDropdownList, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(modelDropdownList, theme->borderColor, LV_PART_MAIN );
        lv_obj_set_style_text_color(modelDropdownList, theme->textColor, LV_PART_MAIN);
        lv_obj_set_style_text_font(modelDropdownList, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_bg_color(modelDropdownList, theme->primaryColor, LV_PART_SELECTED);
        lv_obj_set_style_bg_color(modelDropdownList, theme->primaryColor, LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(modelDropdownList, theme->primaryColor, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(modelDropdownList, theme->backgroundColor, LV_PART_SELECTED | LV_STATE_CHECKED);
        #endif

        // Create save button
        lv_obj_t* saveButton = lv_btn_create(confirmContainer);
        lv_obj_set_size(saveButton, 152, 96);
        lv_obj_align(saveButton, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
        
        // Create label for save button
        lv_obj_t* saveAndRebootLabel = lv_label_create(saveButton);
        lv_label_set_text(saveAndRebootLabel, "SAVE\nAND\nREBOOT");
        lv_obj_set_style_text_align(saveAndRebootLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_center(saveAndRebootLabel);
        
        // Style the button and label
        lv_obj_set_style_bg_color(saveButton, theme->primaryColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(saveButton, LV_OPA_20, LV_PART_MAIN);
        lv_obj_set_style_text_font(saveAndRebootLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(saveAndRebootLabel, theme->textColor, LV_PART_MAIN);
        lvgl_port_unlock();
        // Add event handler
        if (saveButton != NULL) {
            lv_obj_add_event_cb(saveButton, resetAsicSettingsButtonEventHandler, LV_EVENT_CLICKED, NULL);
        }
        

    } else if (!confirmedOverheatMode && overheatOverlay) {
        Serial.println("Removing overlay");
        // Remove overlay when settings are no longer being changed
        lv_obj_del(overheatOverlay);
        overheatOverlay = NULL;
        
    }


}

static void resetAsicSettingsButtonEventHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn = lv_event_get_target(e);
    uiTheme_t* theme = getCurrentTheme();
    if(code == LV_EVENT_CLICKED) {
        #if (BitaxeUltra == 0 && BitaxeSupra == 0 && BitaxeGamma == 0)
        // Get the dropdown object
        lv_obj_t* dropdown = lv_obj_get_parent(btn);  // Get parent container
        dropdown = lv_obj_get_child(dropdown, 1);     // Get the dropdown (adjust index if needed)
        // Get selected option
        char modelBuffer[32];
        lv_dropdown_get_selected_str(dropdown, modelBuffer, sizeof(modelBuffer));
        
        Serial.printf("Selected model: %s\n", modelBuffer);
        #endif
        lvgl_port_lock(-1);
        // Create popup message
        lv_obj_t* popup = lv_obj_create(lv_scr_act());
        lv_obj_set_size(popup, 300, 150);
        lv_obj_center(popup);
        lv_obj_set_style_bg_color(popup, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(popup, theme->borderColor, LV_PART_MAIN);
        lv_obj_set_style_border_width(popup, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(popup, 16, LV_PART_MAIN);
        
        // Add message label
        lv_obj_t* message = lv_label_create(popup);
        lv_label_set_text(message, "Settings Saved\nRebooting...");
        lv_obj_set_style_text_font(message, theme->fontMedium24, LV_PART_MAIN);
        lv_obj_set_style_text_color(message, theme->textColor, LV_PART_MAIN);
        lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_center(message);
        lv_refr_now(NULL);
        lvgl_port_unlock();
        specialRegisters.overheatMode = false;
        // Send flag to BAP to clear overheat mode
        delay(1000);
        if (specialRegisters.overheatMode == false)
        {
            writeDataToBAP(&specialRegisters.overheatMode, 1, LVGL_FLAG_OVERHEAT_MODE);
            delay(5000);
        }
        else
        {
            Serial.println("Overheat mode flag is still true, trying again...");
            specialRegisters.overheatMode = false;
            writeDataToBAP(&specialRegisters.overheatMode, 1, LVGL_FLAG_OVERHEAT_MODE);
            delay(5000);
        }
        #if (BitaxeUltra == 0 && BitaxeSupra == 0 && BitaxeGamma == 0)
        // send data to BAP Voltage first then Frequency
        if (strcmp(modelBuffer, "Bitaxe Gama") == 0)
        {
            uint8_t voltBytes[2] = {    
                (uint8_t)(1050 >> 8),    // High byte for 1050mV
                (uint8_t)(1050 & 0xFF)   // Low byte
            };
            writeDataToBAP(voltBytes, 2, BAP_ASIC_VOLTAGE_BUFFER_REG);
            delay(5000);
            uint8_t freqBytes[2] = {
                (uint8_t)(420 >> 8),    // High byte for 450MHz
                (uint8_t)(420 & 0xFF)   // Low byte
            };
            writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
            delay(1000);

        }
        else if (strcmp(modelBuffer, "Bitaxe Supra") == 0)
        {
            uint8_t voltBytes[2] = {
                (uint8_t)(1150 >> 8),    // High byte for 1150mV
                (uint8_t)(1150 & 0xFF)   // Low byte
            };
            writeDataToBAP(voltBytes, 2, BAP_ASIC_VOLTAGE_BUFFER_REG);
            delay(5000);
            uint8_t freqBytes[2] = {
                (uint8_t)(450 >> 8),    // High byte for 450MHz
                (uint8_t)(450 & 0xFF)   // Low byte
            };
            writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
            delay(1000);

        }
        else if (strcmp(modelBuffer, "Bitaxe Ultra") == 0)
        {
            uint8_t voltBytes[2] = {
                (uint8_t)(1150 >> 8),    // High byte for 1150mV
                (uint8_t)(1150 & 0xFF)   // Low byte
            };
            writeDataToBAP(voltBytes, 2, BAP_ASIC_VOLTAGE_BUFFER_REG);
            delay(5000);
            uint8_t freqBytes[2] = {
                (uint8_t)(450 >> 8),    // High byte for 450MHz
                (uint8_t)(450 & 0xFF)   // Low byte
            };
            writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
            delay(1000);

        }
        #else
        {
            // send data to BAP Voltage first then Frequency
            setNormalPowerPreset();
            uint8_t autotuneEnabledSave = autoTuneEnabled;
            saveSettingsToNVSasU16(NVS_KEY_ASIC_AUTOTUNE_ENABLED, autotuneEnabledSave);
            saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_VOLTAGE, (uint16_t)((BAPAsicVoltageBuffer[0] << 8) | BAPAsicVoltageBuffer[1]));
            writeDataToBAP(BAPAsicVoltageBuffer, 2, BAP_ASIC_VOLTAGE_BUFFER_REG);
            saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_FREQ, (uint16_t)((BAPAsicFreqBuffer[0] << 8) | BAPAsicFreqBuffer[1]));
            writeDataToBAP(BAPAsicFreqBuffer, 2, BAP_ASIC_FREQ_BUFFER_REG);
            saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_FAN_SPEED, (uint16_t)((BAPFanSpeedBuffer[0] << 8) | BAPFanSpeedBuffer[1]));
            writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
            saveSettingsToNVSasU16(NVS_KEY_ASIC_CURRENT_AUTO_FAN_SPEED, (uint16_t)((BAPAutoFanSpeedBuffer[0] << 8) | BAPAutoFanSpeedBuffer[1]));
            writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);

        }
        #endif



        
        delay(2000);
        specialRegisters.restart = 1;
    }
}

lv_obj_t* setupScreenContainer;
void setupSettingsScreen()
{
    uiTheme_t* theme = getCurrentTheme();
    // Setup Screen Container
    setupScreenContainer = lv_obj_create(screenObjs.background);
    lv_obj_set_size(setupScreenContainer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(setupScreenContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_bg_color(setupScreen, lv_color_hex(0x161f1b), LV_PART_MAIN);
    lv_obj_set_style_border_opa(setupScreenContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_color(setupScreenContainer, theme->borderColor, LV_PART_MAIN);
    //lv_obj_set_style_border_width(setupScreenContainer, 2, LV_PART_MAIN);
    //lv_obj_set_style_radius(setupScreenContainer, 16, LV_PART_MAIN);
    lv_obj_clear_flag(setupScreenContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Setup Screen Title
    lv_obj_t* setupScreenTitle = lv_label_create(setupScreenContainer);
    lv_label_set_text(setupScreenTitle, "NETWORK NOT DETECTED\nSETUP NETWORK AND MINING POOL");
    lv_obj_set_style_text_align(setupScreenTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(setupScreenTitle, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(setupScreenTitle, theme->textColor, LV_PART_MAIN);
    lv_obj_align(setupScreenTitle, LV_ALIGN_TOP_MID, 0, 16);

    // Wifi SSID Text Area
    lv_obj_t* wifiTextArea = setTextAreaStyles(setupScreenContainer, "Network SSID");
    lv_textarea_set_max_length(wifiTextArea, MAX_SSID_LENGTH);
    lv_obj_align(wifiTextArea, LV_ALIGN_TOP_LEFT, 0, 96);
    lv_obj_set_width(wifiTextArea, lv_pct(45));
    lv_obj_add_event_cb(wifiTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(wifiTextArea, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(wifiTextArea);

    // Wifi Password Text Area
    lv_obj_t* wifiPasswordTextArea = setTextAreaStyles(setupScreenContainer, "Network Password");
    lv_textarea_set_password_mode(wifiPasswordTextArea, true);
    lv_obj_align (wifiPasswordTextArea, LV_ALIGN_TOP_LEFT, 0, 160);
    lv_obj_set_width(wifiPasswordTextArea, lv_pct(45));
    lv_obj_add_event_cb(wifiPasswordTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(wifiPasswordTextArea, LV_OBJ_FLAG_SCROLLABLE);

    // Stratum URL Main Text Area
    lv_obj_t* stratumUrlTextAreaMain = setTextAreaStyles(setupScreenContainer, "Stratum URL Main");
    lv_obj_align(stratumUrlTextAreaMain, LV_ALIGN_TOP_RIGHT, 0, 96);
    lv_obj_set_width(stratumUrlTextAreaMain, lv_pct(45));
    lv_obj_add_event_cb(stratumUrlTextAreaMain, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(stratumUrlTextAreaMain, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(stratumUrlTextAreaMain);

    //Stratum Port Main Text Area
    lv_obj_t* stratumPortTextAreaMain = setTextAreaStyles(setupScreenContainer, "Stratum Port Main");
    lv_obj_align(stratumPortTextAreaMain, LV_ALIGN_TOP_RIGHT, 0, 160);
    lv_obj_set_width(stratumPortTextAreaMain, lv_pct(45));
    lv_obj_add_event_cb(stratumPortTextAreaMain, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(stratumPortTextAreaMain, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(stratumPortTextAreaMain);

    // Stratum User Main  Text Area
        lv_obj_t* stratumUserTextAreaMain = setTextAreaStyles(setupScreenContainer, "Stratum User Main");
    lv_obj_align(stratumUserTextAreaMain, LV_ALIGN_TOP_RIGHT, 0, 224);
    lv_obj_set_width(stratumUserTextAreaMain, lv_pct(45));
    lv_obj_add_event_cb(stratumUserTextAreaMain, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(stratumUserTextAreaMain, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(stratumUserTextAreaMain);

    // Stratum Password Main Text Area
    lv_obj_t* stratumPasswordTextAreaMain = setTextAreaStyles(setupScreenContainer, "Stratum Password Main");
    lv_obj_align(stratumPasswordTextAreaMain, LV_ALIGN_TOP_RIGHT, 0, 288);
    lv_obj_set_width(stratumPasswordTextAreaMain, lv_pct(45));
    lv_obj_add_event_cb(stratumPasswordTextAreaMain, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(stratumPasswordTextAreaMain, LV_OBJ_FLAG_SCROLLABLE);
    setCursorStyles(stratumPasswordTextAreaMain);
    setCursorStyles(wifiPasswordTextArea);

    // QR Code Container
    lv_obj_t* qrCodeContainer = lv_obj_create(setupScreenContainer);
    lv_obj_set_size(qrCodeContainer, 416, 216);
    lv_obj_set_width(qrCodeContainer, lv_pct(45));
    lv_obj_align(qrCodeContainer, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_opa(qrCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(qrCodeContainer, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_color(qrCodeContainer, theme->borderColor, LV_PART_MAIN);
    //lv_obj_set_style_border_width(qrCodeContainer, 2, LV_PART_MAIN);
    //lv_obj_set_style_radius(qrCodeContainer, 16, LV_PART_MAIN);
    lv_obj_clear_flag(qrCodeContainer, LV_OBJ_FLAG_SCROLLABLE);

    // QR Code Label
    lv_obj_t* qrCodeLabel = lv_label_create(qrCodeContainer);
    lv_label_set_text(qrCodeLabel, "New to Mining?\n Scan QR Code to Get Started!");
    lv_obj_set_style_text_font(qrCodeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(qrCodeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(qrCodeLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(qrCodeLabel, LV_ALIGN_TOP_MID, 0, -8);

    // Setup QR Code
    const char* qrCodeText = "https://www.advancedcryptoservices.com/bitaxe-support";
    lv_obj_t* qrCode = lv_qrcode_create(qrCodeContainer,128,theme->primaryColor,theme->backgroundColor);
    lv_qrcode_update(qrCode, qrCodeText, strlen(qrCodeText));
    lv_obj_align(qrCode, LV_ALIGN_BOTTOM_MID, 0, 8);
       // Keyboard Creation
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
    // Remove all default styles
    lv_obj_remove_style_all(kb);
    // Set keyboard background
    lv_obj_set_style_bg_color(kb, theme->backgroundColor, LV_PART_MAIN);  // Dark background
    lv_obj_set_style_bg_opa(kb, LV_OPA_80, LV_PART_MAIN);
    // Style for buttons (single state only)
    lv_obj_set_style_bg_color(kb, theme->backgroundColor, LV_PART_ITEMS);  // Dark button background
    lv_obj_set_style_text_color(kb, theme->textColor, LV_PART_ITEMS);  // Your theme color for text
    lv_obj_set_style_border_width(kb, 2, LV_PART_ITEMS);
    lv_obj_set_style_border_color(kb, theme->borderColor, LV_PART_ITEMS);
    lv_obj_set_style_radius(kb, 24, LV_PART_ITEMS);
    // Regular keys
    lv_obj_set_style_text_font(kb, theme->fontMedium16, LV_PART_ITEMS);
    // Symbols that Inter Font doesn't support
    lv_obj_set_style_text_font(kb, LV_FONT_DEFAULT, LV_PART_ITEMS | LV_STATE_CHECKED);
    // Disable ALL animations and transitions
    lv_obj_set_style_anim_time(kb, 0, 0);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(kb, wifiTextArea);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);


    // Store text area references as we create them
    settingsTextAreas.wifiTextArea = wifiTextArea;
    settingsTextAreas.wifiPasswordTextArea = wifiPasswordTextArea;
    settingsTextAreas.stratumUrlTextArea = stratumPortTextAreaMain;
    settingsTextAreas.stratumPortTextArea = stratumPortTextAreaMain;
    settingsTextAreas.stratumUserTextArea = stratumUserTextAreaMain;
    settingsTextAreas.stratumPasswordTextArea = stratumPasswordTextAreaMain;
  

    // Create save button
    lv_obj_t* saveButton = lv_btn_create(setupScreenContainer);
    lv_obj_set_size(saveButton, 200, 80);
    lv_obj_align(saveButton, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    
    // Create label for save button
    lv_obj_t* saveLabel = lv_label_create(saveButton);
    lv_label_set_text(saveLabel, "SAVE\nAND\nREBOOT");
    lv_obj_set_style_text_align(saveLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(saveLabel);
    
    // Style the button and label
    lv_obj_set_style_bg_color(saveButton, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(saveButton, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_text_font(saveLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(saveLabel, theme->textColor, LV_PART_MAIN);

    // Add event handler
    if (saveButton != NULL) {
        lv_obj_add_event_cb(saveButton, saveButtonEventHandler, LV_EVENT_CLICKED, NULL);
    }

    // create timer to check for startup done
    lv_timer_create(checkStartupDone, 1000, NULL);

}

static void checkStartupDone(lv_timer_t* timer) {
    if (specialRegisters.startupDone) {
        lvgl_port_lock(-1);
        
        // Delete the timer first
        lv_timer_del(timer);

        // Switch to home screen before cleanup
        switchToScreen(activeScreenHome);
        
        // Show the status bar for main screens
        lv_obj_clear_flag(statusBarObj, LV_OBJ_FLAG_HIDDEN);
        settingsTextAreas.wifiTextArea = NULL;
        settingsTextAreas.wifiPasswordTextArea = NULL;
        settingsTextAreas.stratumUrlTextArea = NULL;
        settingsTextAreas.stratumPortTextArea = NULL;
        settingsTextAreas.stratumUserTextArea = NULL;
        settingsTextAreas.stratumPasswordTextArea = NULL;
        
        // Find and delete the setup screen container

        lv_obj_del(setupScreenContainer);
        
        
        // Load the background screen
        //lv_scr_load(screenObjs.background);
        
        lvgl_port_unlock();
    }
}

// Add this event handler function
static void themeDropdownEventHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {  // Only trigger when dropdown value changes
        char themeBuffer[32] = {0};
        lv_dropdown_get_selected_str(themeDropdown, themeBuffer, sizeof(themeBuffer));
        Serial.printf("Selected theme: %s\n", themeBuffer);
        
        lvgl_port_lock(-1);
        
        if (strcmp(themeBuffer, "BLOCKSTREAM JADE") == 0) {
            #if (BlockStreamJade == 1)
            initializeTheme(THEME_BLOCKSTREAM_JADE);
            #endif
        }
        else if (strcmp(themeBuffer, "BLOCKSTREAM BLUE") == 0) {
            #if (BlockStreamJade == 1)
            initializeTheme(THEME_BLOCKSTREAM_BLUE);
            #endif
        }
        
        else if (strcmp(themeBuffer, "BITAXE RED") == 0) {
            initializeTheme(THEME_BITAXE_RED);
        }
        else if (strcmp(themeBuffer, "ACS DEFAULT") == 0) {
            initializeTheme(THEME_DEFAULT);
        }
        else if (strcmp(themeBuffer, "SOLO SATOSHI") == 0) {
            #if (SoloSatoshi == 1)
            initializeTheme(THEME_SOLO_SATOSHI);
            #endif
        }
        else if (strcmp(themeBuffer, "ALTAIR") == 0) {
            #if (ALTAIR == 1)
            initializeTheme(THEME_ALTAIR);
            #endif
        }
        else if (strcmp(themeBuffer, "SOLO MINING CO") == 0) {
            #if (SoloMiningCo == 1)
            initializeTheme(THEME_SOLO_MINING_CO);
            #endif
        }
        else if (strcmp(themeBuffer, "BTCMAGAZINE") == 0) {
            #if (BTCMagazine == 1)
            initializeTheme(THEME_BTCMAGAZINE);
            #endif
        }
        else if (strcmp(themeBuffer, "VOSKCOIN") == 0) {
            #if (VoskCoin == 1)
            initializeTheme(THEME_VOSKCOIN);
            #endif
        }


        uiTheme_t* theme = getCurrentTheme();
        if (theme) {
            // Set theme container style
            lv_obj_set_style_border_color(themeExampleContainer, theme->borderColor, LV_PART_MAIN);
            lv_obj_set_style_text_font(themePreviewLabel, theme->fontMedium24, LV_PART_MAIN);
            lv_obj_set_style_text_color(themePreviewLabel, theme->textColor, LV_PART_MAIN);
            
            // Update preview image
            lv_img_set_src(themeExampleImage, theme->themePreview);
            Serial0.println("Theme Preview Updated");
            Serial0.println(theme->themePreview);
        }
        
        lvgl_port_unlock();
    }
}

static void timeZoneListEventHandler(lv_event_t* e) {
    lv_obj_t* list = lv_event_get_target(e);
    const char* selectedTz = lv_list_get_btn_text(list, list);
    uiTheme_t* theme = getCurrentTheme();
    static lv_obj_t* saveButton;
    static lv_obj_t* saveButtonLabel;

    lvgl_port_lock(-1);
    // delete custom offset label and text areas
    if (timeOffsetLabel) {
        lv_obj_del_async(timeOffsetLabel);
        timeOffsetLabel = NULL;
    }
    
    if (customOffsetHoursTextArea) {
        lv_obj_del_async(customOffsetHoursTextArea);
        customOffsetHoursTextArea = NULL;
    }
    
    if (customOffsetMinutesTextArea) {
        lv_obj_del_async(customOffsetMinutesTextArea);
        customOffsetMinutesTextArea = NULL;
    }
    
    if (customOffsetLabel) {
        lv_obj_del_async(customOffsetLabel);
        customOffsetLabel = NULL;
    }
    
    if (saveButton) {
        // Button label will be automatically deleted with the button
        lv_obj_del_async(saveButton);
        saveButton = NULL;
        saveButtonLabel = NULL;  // Will be deleted with parent
    }
    lvgl_port_unlock();
    if (strcmp(selectedTz, "Custom Offset") == 0) {
        // TODO: Show custom offset input
        // create label for custom offset
        customOffsetLabel = lv_label_create(timeSettingsContainer);
        lv_label_set_text(customOffsetLabel, "Custom Offset");
        lv_obj_set_style_text_font(customOffsetLabel, theme->fontMedium24, LV_PART_MAIN);
        lv_obj_set_style_text_color(customOffsetLabel, theme->textColor, LV_PART_MAIN);
        lv_obj_align(customOffsetLabel, LV_ALIGN_TOP_RIGHT, -88, -16);
        lv_obj_clear_flag(customOffsetLabel, LV_OBJ_FLAG_SCROLLABLE);

        // create text area for custom offset hours
        customOffsetHoursTextArea = setTextAreaStyles(timeSettingsContainer, "Hours");
        lv_obj_align(customOffsetHoursTextArea, LV_ALIGN_TOP_RIGHT, -16, 16);
        lv_obj_set_width(customOffsetHoursTextArea, lv_pct(38));
        lv_obj_add_event_cb(customOffsetHoursTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_clear_flag(customOffsetHoursTextArea, LV_OBJ_FLAG_SCROLLABLE);
        setCursorStyles(customOffsetHoursTextArea);

        // create text area for custom offset minutes
        customOffsetMinutesTextArea = setTextAreaStyles(timeSettingsContainer, "Minutes");
        lv_obj_align(customOffsetMinutesTextArea, LV_ALIGN_TOP_RIGHT, -16, 80);
        lv_obj_set_width(customOffsetMinutesTextArea, lv_pct(38));
        lv_obj_add_event_cb(customOffsetMinutesTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_clear_flag(customOffsetMinutesTextArea, LV_OBJ_FLAG_SCROLLABLE);
        setCursorStyles(customOffsetMinutesTextArea);

        // create save button
        saveButton = lv_btn_create(timeSettingsContainer);
        lv_obj_set_size(saveButton, 128, 48);
        lv_obj_align(saveButton, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
        lv_obj_set_style_bg_color(saveButton, theme->primaryColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(saveButton, LV_OPA_100, LV_PART_MAIN);
        lv_obj_add_flag(saveButton, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(saveButton, timeZoneSaveButtonEventHandler, LV_EVENT_CLICKED, NULL);

        //save button label
        saveButtonLabel = lv_label_create(saveButton);
        lv_label_set_text(saveButtonLabel, "Save");
        lv_obj_set_style_text_font(saveButtonLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(saveButtonLabel, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_text_opa(saveButtonLabel, LV_OPA_100, LV_PART_MAIN);
        lv_obj_center(saveButtonLabel);
    }
    else if (strcmp(selectedTz, "EST -5") == 0) {
        setTimeOffset(-5, 0);
    }
    else if (strcmp(selectedTz, "CST -6") == 0) {
        setTimeOffset(-6, 0);
    }
    else if (strcmp(selectedTz, "MST -7") == 0) {
        setTimeOffset(-7, 0);
    }
    else if (strcmp(selectedTz, "PST -8") == 0) {
        setTimeOffset(-8, 0);
    }
    else if (strcmp(selectedTz, "AKST -9") == 0) {
        setTimeOffset(-9, 0);
    }
    else if (strcmp(selectedTz, "HST -10") == 0) {
        setTimeOffset(-10, 0);
    }
    else if (strcmp(selectedTz, "GMT +0") == 0) {
        setTimeOffset(0, 0);
    }
    else if (strcmp(selectedTz, "CET +1") == 0) {
        setTimeOffset(1, 0);
    }
    else if (strcmp(selectedTz, "AWST +8") == 0) {
        setTimeOffset(8, 0);
    }
    else if (strcmp(selectedTz, "ACST +9:30") == 0) {
        setTimeOffset(9, 30);
    }
    else if (strcmp(selectedTz, "AEST +10") == 0) {
        setTimeOffset(10, 0);
    }
    else if (strcmp(selectedTz, "ACDT +10:30") == 0) {
        setTimeOffset(10, 30);
    }
    else if (strcmp(selectedTz, "AEDT +11") == 0) {
        setTimeOffset(11, 0);
    }
    else {
        ESP_LOGE("TimeZone", "Invalid time zone selected");
    }
}

static void timeZoneSaveButtonEventHandler(lv_event_t* e) {
    uiTheme_t* theme = getCurrentTheme();
    
    // TODO: Save custom offset
    ESP_LOGI("TimeZone", "Save button clicked");
    ESP_LOGI("TimeZone", "Custom offset hours: %s", lv_textarea_get_text(customOffsetHoursTextArea));
    ESP_LOGI("TimeZone", "Custom offset minutes: %s", lv_textarea_get_text(customOffsetMinutesTextArea));
    // Get the hours and minutes from the text areas
    int hours = atoi(lv_textarea_get_text(customOffsetHoursTextArea));
    int minutes = atoi(lv_textarea_get_text(customOffsetMinutesTextArea));
    // Set Time Offset
    setTimeOffset(hours, minutes);

    // create string for label text
    char labelText[32];
    sprintf(labelText, "Time Offset:\n%d hours:%d minutes", hours, minutes);

    // create a label to show the time offset
    lvgl_port_lock(-1);
    if (timeOffsetLabel != NULL) {
        lv_obj_del(timeOffsetLabel);
    }
    timeOffsetLabel = lv_label_create(timeSettingsContainer);
    lv_label_set_text(timeOffsetLabel, labelText);
    lv_obj_set_style_text_font(timeOffsetLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(timeOffsetLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(timeOffsetLabel, LV_ALIGN_TOP_RIGHT, -16, 144);
    lv_obj_clear_flag(timeOffsetLabel, LV_OBJ_FLAG_SCROLLABLE);
    lvgl_port_unlock();
}

static void overheatModeButtonEventHandler(lv_event_t* e) {
    // Toggle Overheat mode
   specialRegisters.overheatMode = 1;                       
   ESP_LOGI("Overheat Mode", "Overheat mode enabled");
   confirmedOverheatMode = true;
}

static void blockFoundButtonEventHandler(lv_event_t* e) {
    // Toggle Block Found
    specialRegisters.foundBlock = 1;
    ESP_LOGI("Block Found", "Block Found");
    confirmedFoundBlock = true;
}

void showBlockFoundOverlay() {
 
    uiTheme_t* theme = getCurrentTheme();
    // Get mempool state pointer
    MempoolApiState* mempoolState = getMempoolState();
    static lv_obj_t* blockFoundOverlay = NULL;

   // create overlay
    if (confirmedFoundBlock && !blockFoundOverlay) 
    {
        lvgl_port_lock(-1);
        Serial.println("Creating overlay");
        // Create semi-transparent overlay
        blockFoundOverlay = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(blockFoundOverlay);
        lv_obj_set_size(blockFoundOverlay, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(blockFoundOverlay, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(blockFoundOverlay, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_radius(blockFoundOverlay, 0, LV_PART_MAIN);
        lv_obj_clear_flag(blockFoundOverlay, LV_OBJ_FLAG_SCROLLABLE);
        
        // Create confirmation container
        lv_obj_t* confirmContainer = lv_obj_create(blockFoundOverlay);
        lv_obj_set_size(confirmContainer, 672, 392);
        lv_obj_align(confirmContainer, LV_ALIGN_CENTER, 40, 28);
        lv_obj_set_style_bg_color(confirmContainer, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(confirmContainer, theme->borderColor, LV_PART_MAIN);
        lv_obj_set_style_border_width(confirmContainer, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(confirmContainer, 16, LV_PART_MAIN);
        lv_obj_clear_flag(confirmContainer, LV_OBJ_FLAG_SCROLLABLE);
        
        // Add Title
        lv_obj_t* title = lv_label_create(confirmContainer);
        lv_label_set_text(title, "BLOCK FOUND!");
        lv_obj_set_style_text_font(title, theme->fontExtraBold56, LV_PART_MAIN);
        lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(title, theme->textColor, LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

        // get variable for current block height and difficulty
        uint32_t blockHeight = mempoolState->blockHeight;
        char* difficulty = IncomingData.mining.bestDiff;

        // create string for message text   
        char messageText[100];
        sprintf(messageText, "This Bitaxe solved the solution for\nBlock Height:\n %ld\n With a Difficulty of:\n %s", blockHeight, difficulty);

        // Add message
        lv_obj_t* message = lv_label_create(confirmContainer);
        lv_label_set_text(message, messageText);
        lv_obj_set_style_text_font(message, theme->fontExtraBold32, LV_PART_MAIN);
        lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(message, theme->textColor, LV_PART_MAIN);
        lv_obj_align(message, LV_ALIGN_TOP_MID, 0, 72);

        // Dismiss button
        lv_obj_t* dismissButton = lv_btn_create(confirmContainer);
        lv_obj_set_size(dismissButton, 128, 48);
        lv_obj_align(dismissButton, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
        lv_obj_set_style_bg_color(dismissButton, theme->primaryColor, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(dismissButton, LV_OPA_100, LV_PART_MAIN);
        lv_obj_add_event_cb(dismissButton, dismissBlockFoundButtonEventHandler, LV_EVENT_CLICKED, NULL);

        // Dismiss button label
        lv_obj_t* dismissButtonLabel = lv_label_create(dismissButton);
        lv_label_set_text(dismissButtonLabel, "Dismiss");
        lv_obj_set_style_text_font(dismissButtonLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(dismissButtonLabel, theme->backgroundColor, LV_PART_MAIN);
        lv_obj_set_style_text_opa(dismissButtonLabel, LV_OPA_100, LV_PART_MAIN);
        lv_obj_center(dismissButtonLabel);

        // qr code for mempool block height

         // mempool QR Code Container
        lv_obj_t* mempoolQRCodeContainer = lv_obj_create(confirmContainer);
        lv_obj_set_size(mempoolQRCodeContainer, 200, 232);
        lv_obj_align(mempoolQRCodeContainer, LV_ALIGN_BOTTOM_LEFT, 0, 32);
        lv_obj_set_style_bg_opa(mempoolQRCodeContainer, LV_OPA_0, LV_PART_MAIN);
        lv_obj_set_style_border_opa(mempoolQRCodeContainer, LV_OPA_0, LV_PART_MAIN);
        lv_obj_clear_flag(mempoolQRCodeContainer, LV_OBJ_FLAG_SCROLLABLE);
        // URL String
        char urlString[100];
        String mempoolUrl = "https://mempool.space/block/";
        snprintf(urlString, sizeof(urlString), "%s%ld", mempoolUrl.c_str(), blockHeight + 1); // +1 because API Server wont update in time

        // mempool QR Code
        lv_obj_t* mempoolQRCode = lv_qrcode_create(mempoolQRCodeContainer, 128, theme->primaryColor, theme->backgroundColor);
        if (WiFi.status() == WL_CONNECTED) 
        {
        lv_qrcode_update(mempoolQRCode, urlString, strlen(urlString));
        }
        lv_obj_align(mempoolQRCode, LV_ALIGN_BOTTOM_LEFT, -16, -16);

        // mempool Address Label
        char mempoolAddressLabelText[100];
        snprintf(mempoolAddressLabelText, sizeof(mempoolAddressLabelText), "See Details\non Mempool:");
        lv_obj_t* mempoolAddressLabel = lv_label_create(mempoolQRCodeContainer);
        lv_label_set_text(mempoolAddressLabel, mempoolAddressLabelText);
        lv_obj_set_style_text_font(mempoolAddressLabel, theme->fontMedium16, LV_PART_MAIN);
        lv_obj_set_style_text_color(mempoolAddressLabel, theme->textColor, LV_PART_MAIN);
        lv_obj_align(mempoolAddressLabel, LV_ALIGN_TOP_LEFT, -8, 0);
        lv_obj_clear_flag(mempoolAddressLabel, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_text_align(mempoolAddressLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lvgl_port_unlock();
    }
    else if (!confirmedFoundBlock && blockFoundOverlay) 
    {
        Serial.println("Removing overlay");
        // Remove overlay when settings are no longer being changed
        lv_obj_del(blockFoundOverlay);
        blockFoundOverlay = NULL;
        
    }
}

static void dismissBlockFoundButtonEventHandler(lv_event_t* e) {
    // Toggle Overheat mode
    specialRegisters.foundBlock = 0;
    ESP_LOGI("Block Found", "Block Found Screen Dismissed");
}
