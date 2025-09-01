#include "NVS.h"
#include "Arduino.h"
#include <nvs_flash.h>
#include <nvs.h>
#include "UIThemes.h"
#include <SPIFFS.h>

void initializeNVS(void);
themePreset_t loadThemeFromNVS(void);
void saveThemeToNVS(themePreset_t preset);


void initializeNVS(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

themePreset_t loadThemeFromNVS(void) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_THEMES, NVS_READONLY, &nvsHandle);

    // Initialize theme based on available themes
    uint8_t themeValue = 0; // Default to ACS theme
    
    // Check for available themes in order of preference
    if (SPIFFS.exists("/UILogoJADE.png")) {
        themeValue = 2; // Blockstream Jade theme
    } 

    if (err == ESP_OK) {
        err = nvs_get_u8(nvsHandle, NVS_KEY_THEME, &themeValue);
        nvs_close(nvsHandle);
    }
    
    return (themePreset_t)themeValue;
}

void saveThemeToNVS(themePreset_t preset) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_THEMES, NVS_READWRITE, &nvsHandle);
    
    if (err == ESP_OK) {
        err = nvs_set_u8(nvsHandle, NVS_KEY_THEME, (uint8_t)preset);
        if (err == ESP_OK) {
            nvs_commit(nvsHandle);
        }
        nvs_close(nvsHandle);
    }
}

bool isFirstBoot(void) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_FLAGS, NVS_READONLY, &nvsHandle);
    
    uint8_t firstBoot = 1; // Default to true if not found
    Serial0.printf("isFirstBoot: %d\n", firstBoot);
    if (err == ESP_OK) {
        err = nvs_get_u8(nvsHandle, NVS_KEY_FIRST_BOOT, &firstBoot);
        nvs_close(nvsHandle);
    }
    Serial0.printf("isFirstBoot: %d\n", firstBoot);
    return firstBoot;
}

void setFirstBootComplete(void) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_FLAGS, NVS_READWRITE, &nvsHandle);
    
    if (err == ESP_OK) {
        err = nvs_set_u8(nvsHandle, NVS_KEY_FIRST_BOOT, 0);
        if (err == ESP_OK) {
            nvs_commit(nvsHandle);
        }
        nvs_close(nvsHandle);
    }
    Serial0.printf("setFirstBootComplete: %d\n", 0);
}

void saveSettingsToNVSasString(const char* key, const char* value, size_t size)  {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_SETTINGS, NVS_READWRITE, &nvsHandle);

    if (err == ESP_OK) {
        err = nvs_set_str(nvsHandle, key, value);
        if (err == ESP_OK) {
            nvs_commit(nvsHandle);
        }
        nvs_close(nvsHandle);
    }
    ESP_LOGI("NVS", "saveSettingsToNVSasString: %s: %s", key, value);
}

void loadSettingsFromNVSasString(const char* key, char* value, size_t size) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_SETTINGS, NVS_READONLY, &nvsHandle);
    
    // Set default value (empty string) if read fails
    value[0] = '\0';
    
    if (err == ESP_OK) {
        err = nvs_get_str(nvsHandle, key, value, &size);
        nvs_close(nvsHandle);
    }
    ESP_LOGI("NVS", "loadSettingsFromNVSasString: %s: %s", key, value);
}

void saveSettingsToNVSasU16(const char* key, uint16_t value) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_SETTINGS, NVS_READWRITE, &nvsHandle);
    ESP_LOGI("NVS", "Attempting to save U16 %s: %d", key, value);

    if (err == ESP_OK) {
        err = nvs_set_u16(nvsHandle, key, value);
        if (err == ESP_OK) {
            err = nvs_commit(nvsHandle);
            if (err != ESP_OK) {
                ESP_LOGE("NVS", "Failed to commit: %s", esp_err_to_name(err));
            }
        } else {
            ESP_LOGE("NVS", "Failed to set U16: %s", esp_err_to_name(err));
        }
        nvs_close(nvsHandle);
        
        // Verify the save
        uint16_t verify = loadSettingsFromNVSasU16(key);
        if (verify != value) {
            ESP_LOGE("NVS", "Verification failed. Saved: %d, Read: %d", value, verify);
        }
    } else {
        ESP_LOGE("NVS", "Failed to open NVS handle: %s", esp_err_to_name(err));
    }
}

uint16_t loadSettingsFromNVSasU16(const char* key) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_SETTINGS, NVS_READONLY, &nvsHandle);
    
    uint16_t result = 0;
    
    if (err == ESP_OK) {
        err = nvs_get_u16(nvsHandle, key, &result);
        if (err != ESP_OK) {
            ESP_LOGE("NVS", "Failed to read U16: %s", esp_err_to_name(err));
        }
        nvs_close(nvsHandle);
    } else {
        ESP_LOGE("NVS", "Failed to open NVS handle: %s", esp_err_to_name(err));
    }
    ESP_LOGI("NVS", "Loading U16 %s: %d", key, result);
    return result;
}

void factoryResetNVS(void) {
    nvs_flash_erase();
    initializeNVS();
    delay(20);
    esp_restart();
}
