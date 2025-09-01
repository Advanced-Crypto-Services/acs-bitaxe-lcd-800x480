#include "modelPresets.h"
#include "BAP.h"
#include "modelConfig.h"
#include "NVS.h"

#define minPresetTemp  60
#define maxPresetTemp  65 

bool autoTuneEnabled = true;
uint16_t currentPresetFrequency = 0;
uint16_t currentPresetVoltage = 0;
uint8_t currentPresetFanSpeed = 0;
bool currentPresetAutoFanMode = false;
uint16_t autotuneTempLowTarget = 0;
uint16_t autotuneTempHighTarget = 0;

char presetName[10] = "quiet";



// preset autotuning
uint16_t frequencyOffset = 0;
uint16_t voltageOffset = 0;
uint16_t fanSpeedOffset = 0; 


void setQuietPreset() 
{
    strcpy(presetName, "quiet");
    saveSettingsToNVSasString(NVS_KEY_ASIC_PRESET_NAME, presetName, sizeof(presetName));
    writeDataToBAP((uint8_t*)presetName, sizeof(presetName), LVGL_REG_SPECIAL_PRESET);
}


void setBalancedPreset() 
{
    strcpy(presetName, "balanced");
    saveSettingsToNVSasString(NVS_KEY_ASIC_PRESET_NAME, presetName, sizeof(presetName));
    writeDataToBAP((uint8_t*)presetName, sizeof(presetName), LVGL_REG_SPECIAL_PRESET);
}

void setTurboPreset() 
{
    strcpy(presetName, "turbo");
    saveSettingsToNVSasString(NVS_KEY_ASIC_PRESET_NAME, presetName, sizeof(presetName));
    writeDataToBAP((uint8_t*)presetName, sizeof(presetName), LVGL_REG_SPECIAL_PRESET);
}


