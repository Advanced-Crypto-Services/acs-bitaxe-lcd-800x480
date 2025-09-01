#include "wifiFeatures.h"
#include "initialSetupScreen.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <NVS.h>
#include "BAP.h"
#include "Clock.h"
#include "freertos/semphr.h"


#define MAX_NETWORKS 20
#define MAX_SSID_SCAN_LENGTH 33

// Static PSRAM array to store networks
WifiNetworkScan* storedNetworks = nullptr;
uint16_t* storedNetworkCount = nullptr;

extern SemaphoreHandle_t wifiMutex;

void inistialiseWifiScanBuffers() {
    storedNetworks = (WifiNetworkScan*)heap_caps_malloc(
            sizeof(WifiNetworkScan) * MAX_NETWORKS, MALLOC_CAP_SPIRAM);
    storedNetworkCount = (uint16_t*)heap_caps_malloc(
            sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void listNetworks() {
    // Start WiFi scan
    if (wifiMutex)xSemaphoreTake(wifiMutex, portMAX_DELAY);
    int numNetworks = WiFi.scanNetworks();
    
    if (numNetworks == -1) {
        Serial.println("WiFi scan failed!");
        if (wifiMutex)xSemaphoreGive(wifiMutex);
        return;
    }
    
    // Allocate storage for network count if not already allocated
    if (storedNetworkCount == nullptr) {
        storedNetworkCount = (uint16_t*)heap_caps_malloc(
            sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!storedNetworkCount) {
            Serial.println("Failed to allocate network count storage!");
            if (wifiMutex)xSemaphoreGive(wifiMutex);
            return;
        }
    }
    
    // Allocate storage in PSRAM for discovered networks if not already allocated
    if (storedNetworks == nullptr) {
        storedNetworks = (WifiNetworkScan*)heap_caps_malloc(
            sizeof(WifiNetworkScan) * MAX_NETWORKS, MALLOC_CAP_SPIRAM);
        if (!storedNetworks) {
            Serial.println("Failed to allocate network storage in PSRAM!");
            if (wifiMutex)xSemaphoreGive(wifiMutex);
            return;
        }
    }
    
    // Store the actual number of networks found (limited by MAX_NETWORKS)
    memset(storedNetworkCount, 0, sizeof(uint16_t));
    *storedNetworkCount = min(numNetworks, MAX_NETWORKS);
    
    Serial.printf("Number of networks found: %d\n", *storedNetworkCount);
    
    for (int i = 0; i < *storedNetworkCount; i++) {
        //clear the ssid buffer
        memset(storedNetworks[i].ssid, 0, sizeof(storedNetworks[i].ssid));
        // Copy network info to PSRAM storage
        strncpy(storedNetworks[i].ssid, WiFi.SSID(i).c_str(), MAX_SSID_SCAN_LENGTH - 1);
        storedNetworks[i].ssid[MAX_SSID_SCAN_LENGTH - 1] = '\0';
        storedNetworks[i].rssi = WiFi.RSSI(i);
        
        // Print the network info
        Serial0.printf("%2d | SSID: %-32s | RSSI: %ld\n", 
            i + 1, storedNetworks[i].ssid, storedNetworks[i].rssi);
    }
    
    // Clean up scan results
    WiFi.scanDelete();

    Serial0.printf("Free heap: %lu\n", ESP.getFreeHeap());
    Serial0.printf("Minimum free heap: %lu\n", ESP.getMinFreeHeap());
    if (wifiMutex)xSemaphoreGive(wifiMutex);
}

void reconnectWifi() {
    if (wifiMutex)xSemaphoreTake(wifiMutex, portMAX_DELAY);
     static uint32_t lastWifiCheck = 0;
     const char* TAG = "WiFi";
     static char wifiSSID1[MAX_SSID_LENGTH];
     static char wifiPassword1[MAX_SSID_LENGTH];
    
    if (WiFi.status() != WL_CONNECTED && (millis() - lastWifiCheck > 30000)) {  // Check every 30 seconds
        Serial0.println("WiFi connection lost, attempting to reconnect...");
        ESP_LOGI(TAG, "WiFi connection lost, attempting to reconnect...");
        
        char wifiSSID1[MAX_SSID_LENGTH];
        char wifiPassword1[MAX_SSID_LENGTH];
        
        // Load credentials from NVS
        loadSettingsFromNVSasString(NVS_KEY_WIFI_SSID1, wifiSSID1, MAX_SSID_LENGTH);
        loadSettingsFromNVSasString(NVS_KEY_WIFI_PASSWORD1, wifiPassword1, MAX_SSID_LENGTH);
        
        if (strlen(wifiSSID1) > 0 && strlen(wifiPassword1) > 0) {
            WiFi.disconnect();
            delay(10);
            WiFi.begin(wifiSSID1, wifiPassword1);
            
            // Wait briefly for connection
            unsigned long startTime = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 5000)) {
                delay(100);
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                ESP_LOGI(TAG, "WiFi reconnected successfully");
                //espTime();
            } else {
                ESP_LOGI(TAG, "WiFi reconnection failed");
            }
        }
        lastWifiCheck = millis();
    }
    if (wifiMutex)xSemaphoreGive(wifiMutex);
}
