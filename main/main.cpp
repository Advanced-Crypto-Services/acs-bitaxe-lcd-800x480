#include "Arduino.h"
#include <lvgl.h>
#include <lv_conf.h>
#include "lvgl_port_v8.h"
#include <demos/lv_demos.h>
#include <ESP_Panel_Library.h>

#include <SPIFFS.h>
#include <esp_heap_caps.h>
#include "esp_task_wdt.h"
#include "UIThemes.h"
#include <nvs_flash.h>
#include <nvs.h>

#include "spiffsDriver.h"

#include <lv_conf.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>
#include "I2CData.h"
#include <TimeLib.h>

#include <esp_wifi.h>
#include "esp_event.h"
#include <WiFi.h>
#include <WebServer.h>
#include "modelPresets.h"


#include <lvgl.h>
#include "lvgl_port_v8.h"
#include <demos/lv_demos.h>
#include "UIScreens.h"
#include "UIHandler.h"
#include "BAP.h"
#include "NVS.h"
#include "initialSetupScreen.h"
#include "mempoolAPI.h"
#include "wifiFeatures.h"
#include "Clock.h"
#include "httpServer.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include "driver/ledc.h"
#include "lcdSettings.h"





void setBrightness(uint8_t value);

void printMemoryInfo() 
{
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    Serial0.printf("PSRAM Info:\n");
    Serial0.printf("Total Size: %u bytes\n", info.total_free_bytes + info.total_allocated_bytes);
    Serial0.printf("Free: %u bytes\n", info.total_free_bytes);
    Serial0.printf("Allocated: %u bytes\n", info.total_allocated_bytes);
    
    // For regular heap
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
    Serial0.printf("\nInternal Memory Info:\n");
    Serial0.printf("Total Free: %u bytes\n", info.total_free_bytes);
    Serial0.printf("Total Allocated: %u bytes\n", info.total_allocated_bytes);
}

void listSpiffsContents() {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
        Serial0.printf("File: %s, size: %d\n", file.name(), file.size());
        file = root.openNextFile();
    }
}


void setBrightness(uint8_t value) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    ESP_LOGI("Backlight", "Set brightness to %d", value);
}



extern "C" void app_main()
{
    initArduino();
    Serial.begin(115200);
    delay(100);
        // Initalize BAP Buffers at Runtime
    initializeBAPBuffers();
    Serial0.println("BAP Buffers initialized");
    initializeMempoolState();
    Serial0.println("Mempool State initialized");
    initializeResponseBuffer();
    Serial0.println("Response Buffer initialized");
    //inistialiseWifiScanBuffers();


    // Initialize BAP
    setupBAP();
    Serial0.println("BAP Initialized");
    delay(20);
 
    // Initialize IO Expander
    initializeIOExpander();

    //Initialize Panel
    Serial.println("Initialize panel device");
    ESP_Panel *panel = new ESP_Panel();
    panel->init();
#if LVGL_PORT_AVOID_TEAR
    // When avoid tearing function is enabled, configure the RGB bus according to the LVGL configuration
    ESP_PanelBus_RGB *rgb_bus = static_cast<ESP_PanelBus_RGB *>(panel->getLcd()->getBus());
    rgb_bus->configRgbFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
    rgb_bus->configRgbBounceBufferSize(LVGL_PORT_RGB_BOUNCE_BUFFER_SIZE);
#endif
    panel->begin();
    delay (20);
  Serial0.println("Initialize LVGL");
    lvgl_port_init(panel->getLcd(), panel->getTouch());

    lv_img_cache_set_size(LV_IMG_CACHE_DEF_SIZE);
    
    // Initialize PNG decoder
    lv_png_init();
    Serial0.println("PNG decoder initialized");
    initSpiffsDriver();
    listSpiffsContents();

    initializeNVS();
    delay(20);
    Serial0.println("NVS initialized");
    // Initialize UI with backlight off
    initializeTheme(loadThemeFromNVS());
    Serial0.println("Theme initialized");
    Serial0.printf("Theme: %d\n", loadThemeFromNVS());
    delay(20);

    WiFi.mode(WIFI_STA);
    // Scan networks
    Serial0.println("Scanning WiFi networks...");
    splashScreen();
    setupWebServer();
    listNetworks();
    if (isFirstBoot()) 
        {
        
        delay(20);
        
        delay(20);
        initialSetupScreen();
        }
    

    char wifiSSID1[MAX_SSID_LENGTH];
    char wifiPassword1[MAX_SSID_LENGTH];
    // Load from NVS
    loadSettingsFromNVSasString(NVS_KEY_WIFI_SSID1, wifiSSID1, MAX_SSID_LENGTH);
    loadSettingsFromNVSasString(NVS_KEY_WIFI_PASSWORD1, wifiPassword1, MAX_SSID_LENGTH);

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial0.println("WiFi already connected");
        WiFi.disconnect();
        delay(20);
        WiFi.mode(WIFI_OFF);
        delay(20);
        WiFi.mode(WIFI_STA);
    }

    if (strlen(wifiSSID1) > 0 && strlen(wifiPassword1) > 0 ) 
    {
        Serial0.println("Loading WiFi settings from NVS");
        Serial0.printf("SSID: %s\n", wifiSSID1);
        Serial0.printf("Password: %s\n", wifiPassword1);
        WiFi.begin(wifiSSID1, wifiPassword1);
        Serial0.println("WiFi settings loaded from NVS");

        // Wait for WiFi connection with timeout
         unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startTime > 10000) {
                Serial0.println("WiFi connection timed out");
                WiFi.disconnect();
                delay(20);
                WiFi.mode(WIFI_OFF);
                delay(20);
                WiFi.mode(WIFI_STA);
                break;
                }
            Serial0.println("Waiting for WiFi connection...");
            delay(100);
            }
            Serial0.println("WiFi connected");
    }


    //splashScreen();
    delay(20);
    // Turn on backlight
    //expander->digitalWrite(LCD_BL, HIGH);
    //delay(1000);
/*
    uint32_t startTime = millis();
    while (millis() - startTime < 10000)  // Wait for 10 seconds
    {
        readDataFromBAP();
        delay(100);  // Small delay to prevent tight looping

        if (specialRegisters.startupDone == 1)
        {
            initalizeOneScreen();
            switchToScreen(activeScreenHome);
            delay(20);
            
            break;
        }
    }
    Serial0.println("Startup Done");
    
    // After 10 seconds, check startup flag
    if (specialRegisters.startupDone == 0)
    {
        initalizeOneScreen();
        delay(20);
        Serial0.println("No Network Detected, Setting up Settings Screen");
        switchToScreen(activeScreenHome);
    }
    */
   readCurrentPresetSettingsFromNVS();
   initalizeOneScreen();
   switchToScreen(activeScreenHome);
   
    
    Serial0.println("LVGL porting example end");

    // After LVGL and PNG decoder init
    Serial0.printf("Image cache size: %d KB\n", LV_IMG_CACHE_DEF_SIZE / 1024);

    // After LVGL initialization
    Serial0.printf("Image cache configured for %.2f KB\n", 
        (float)LV_IMG_CACHE_DEF_SIZE / 1024.0);

        delay(20);
            if (WiFi.status() == WL_CONNECTED)
    {
        mempool_api_network_summary();
    }

    espTime();
/*
    // One-time sweep test (if that's what you want)
    if (backlightPWM) {
        // Sweep up
        for (uint8_t i = 1; i <= 254; i++) {
            setBrightness(i);
            ESP_LOGI("Backlight", "PWM Value: %d", i);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Sweep down
        for (uint8_t i = 254; i >= 1; i--) {
            setBrightness(i);
            ESP_LOGI("Backlight", "PWM Value: %d", i);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        // Set final brightness
        setBrightness(25);  // Or whatever final brightness you want
    }
*/
    //main loop
while (true)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            mempool_api_network_summary();// This has its own delay
        }
        setupServer->handleClient();
        vTaskDelay(50);
        //checkI2CHealth();
    static uint32_t lastBAPRead = 0;
    if (millis() - lastBAPRead > 10)
    {
        readDataFromBAP();
        lastBAPRead = millis();
    }

    static uint32_t lastHeapCheck = 0;
    static uint32_t lastClockSync = 0;
    // Monitor heap every second
    if (millis() - lastHeapCheck > 1000) 
    {
        //monitorHeapAllocation();
        printMemoryInfo();
       // lastHeapCheck = millis();
       lastHeapCheck = millis();
    }

    if (millis() - lastClockSync > 1000) 
    {
        keepClockTime();
        lastClockSync = millis();
        Serial0.printf("Clock synced to: %02d/%02d/%04d %02d:%02d:%02d\n", 
            month(), day(), year(),
            hour(), minute(), second());
    }

    static uint32_t lastFlagCheck = 0;
    if (millis() - lastFlagCheck > 1000) // Check once per second
    {
        if (specialRegisters.restart == 1)
        {
            sendRestartToBAP();
        }
        showOverheatOverlay();
        showBlockFoundOverlay();
            lastFlagCheck = millis();
        
    }
        
    static uint32_t lastAutoTuneCheck = 0;
    static bool firstAutoTune = true;
    float asicTemp = IncomingData.monitoring.temperatures[0];
    
    if (millis() - lastAutoTuneCheck > (firstAutoTune ? 600000 : 120000))  // 10 min for first check so bitaxe stabilizes, 60 sec after.
    {
        ESP_LOGI("AutoTune", "Autotune Timer Expired"); 
        presetAutoTune();
        lastAutoTuneCheck = millis();
        firstAutoTune = false;
    }
    if (asicTemp >= 67 && (millis() - lastAutoTuneCheck > 1000)) { //  Instantly if temp is too high
        ESP_LOGI("AutoTune", "Asic Temp: %.2f", asicTemp);
        ESP_LOGI("AutoTune", "Asic Temp Overhead. Tweaking Settings Temp: %.2f", asicTemp);
        presetAutoTune(); 
        lastAutoTuneCheck = millis();
    }
    reconnectWifi();
    }

   

}