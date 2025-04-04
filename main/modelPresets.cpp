#include "modelPresets.h"
#include "BAP.h"
#include "I2CData.h"
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



#if (BitaxeGamma == 1)
const uint16_t voltageLowPower = 1030;
const uint16_t voltageNormalPower = 1090;
const uint16_t voltageHighPower = 1150;

const uint16_t freqLowPower = 420;
const uint16_t freqNormalPower = 490;
const uint16_t freqHighPower = 700;

const uint8_t fanSpeedLowPower = 27;
const uint8_t fanSpeedNormalPower = 35;
const uint8_t fanSpeedHighPower = 80;

const uint8_t autoFanModeLowPower = 0;
const uint8_t autoFanModeNormalPower = 0;
const uint8_t autoFanModeHighPower = 0;
#endif

#if (BitaxeUltra == 1)
const uint16_t voltageLowPower = 1130;
const uint16_t voltageNormalPower = 1190;
const uint16_t voltageHighPower = 1250;

const uint16_t freqLowPower = 420;
const uint16_t freqNormalPower = 490;
const uint16_t freqHighPower = 625;

const uint8_t fanSpeedLowPower = 27;
const uint8_t fanSpeedNormalPower = 35;
const uint8_t fanSpeedHighPower = 80;

const uint8_t autoFanModeLowPower = 0;
const uint8_t autoFanModeNormalPower = 0;
const uint8_t autoFanModeHighPower = 0;
#endif

// preset autotuning
uint16_t frequencyOffset = 0;
uint16_t voltageOffset = 0;
uint16_t fanSpeedOffset = 0; 


void setLowPowerPreset() 
{
 Serial.println("Low Power Mode Selected");

    #if (BitaxeGamma == 1 && BAPPORT == 1)
                // fan speed
                memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                delay(10);
                BAPFanSpeedBuffer[0] = 0x00;
                BAPFanSpeedBuffer[1] = 27; // 27%
                // auto fan speed
                
                BAPAutoFanSpeedBuffer[0] = 0x00;
                BAPAutoFanSpeedBuffer[1] = 0x00; // Auto Fan Speed off
                currentPresetFanSpeed = 27;
                currentPresetAutoFanMode = 0;
                delay(10);
                autoTuneEnabled = 1;

                // asic voltage
                 
                uint16_t voltageNumber = voltageLowPower;  // Convert string to number
                uint8_t voltageBytes[2] = {
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                currentPresetVoltage = voltageLowPower;
                // asic freq
                
                uint16_t freqNumber = freqLowPower;  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                currentPresetFrequency = freqLowPower;
                //writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
                delay(10);

       
                //writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
                //writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);
    #elif (BitaxeUltra == 1 && BAPPORT == 1)
// fan speed
                memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                delay(10);
                BAPFanSpeedBuffer[0] = 0x00;
                BAPFanSpeedBuffer[1] = fanSpeedLowPower; // 27%
                // auto fan speed
                
                BAPAutoFanSpeedBuffer[0] = 0x00;
                BAPAutoFanSpeedBuffer[1] = 0x00; // Auto Fan Speed off
                currentPresetFanSpeed = 27;
                currentPresetAutoFanMode = 0;
                autoTuneEnabled = 1;


                // asic voltage
                 
                uint16_t voltageNumber = voltageLowPower;  // Convert string to number
                uint8_t voltageBytes[2] = {
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                currentPresetVoltage = voltageLowPower;
                // asic freq
                
                uint16_t freqNumber = freqLowPower;  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                currentPresetFrequency = freqLowPower;
                //writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
                delay(10);

       
                //writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
                //writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);
    #endif
}


void setNormalPowerPreset() 
{
    #if (BitaxeGamma == 1 && BAPPORT == 1)
     Serial.println("Normal Power Mode Selected");
                // fan speed
                memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                delay(10);
                BAPFanSpeedBuffer[0] = 0x00;
                BAPFanSpeedBuffer[1] = fanSpeedNormalPower; // 35%   
                // auto fan speed
                
                BAPAutoFanSpeedBuffer[0] = 0x00;
                BAPAutoFanSpeedBuffer[1] = autoFanModeNormalPower; // Auto Fan Speed off
                delay(10);
                currentPresetFanSpeed = 35;
                delay(10);
                currentPresetAutoFanMode = 0;
                delay(10);
                autoTuneEnabled = 1;

                // asic voltage
                uint16_t voltageNumber = voltageNormalPower;  // Convert string to number
                uint8_t voltageBytes[2] = { 
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                currentPresetVoltage = voltageNormalPower;


                // asic freq
                
                uint16_t freqNumber = freqNormalPower;  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                currentPresetFrequency = freqNormalPower;
                //writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
                delay(10);


                //writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
                //writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);
    #elif (BitaxeUltra == 1 && BAPPORT == 1)
     Serial.println("Normal Power Mode Selected");
                // fan speed
                memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                delay(10);
                BAPFanSpeedBuffer[0] = 0x00;
                BAPFanSpeedBuffer[1] = fanSpeedNormalPower; // 35%   
                // auto fan speed
                
                BAPAutoFanSpeedBuffer[0] = 0x00;
                BAPAutoFanSpeedBuffer[1] = autoFanModeNormalPower; // Auto Fan Speed off
                currentPresetFanSpeed = 35;
                currentPresetAutoFanMode = 0;
                autoTuneEnabled = 1;


                // asic voltage
                uint16_t voltageNumber = voltageNormalPower;  // Convert string to number
                uint8_t voltageBytes[2] = { 
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                currentPresetVoltage = voltageNormalPower;
                // asic freq
                
                uint16_t freqNumber = freqNormalPower;  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                currentPresetFrequency = freqNormalPower;
                //writeDataToBAP(freqBytes, 2, BAP_ASIC_FREQ_BUFFER_REG);
                delay(10);


                //writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
                //writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);

    #endif
}

void setHighPowerPreset() 
{
    #if (BitaxeGamma == 1 && BAPPORT == 1)
    Serial.println("High Power Mode Selected");
                // fan speed
                memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                delay(10);

                BAPFanSpeedBuffer[0] = 0x00;
                BAPFanSpeedBuffer[1] = 75; // 75%
                BAPAutoFanSpeedBuffer[0] = 0x00;
                BAPAutoFanSpeedBuffer[1] = autoFanModeHighPower; // Auto Fan Speed
                delay(10);
                currentPresetFanSpeed = 75;
                delay(10);
                currentPresetAutoFanMode = 0;
                delay(10);
                autoTuneEnabled = 1;
                //writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);
                // asic voltage
                uint16_t voltageNumber = voltageHighPower;  // Convert string to number
                uint8_t voltageBytes[2] = {
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                currentPresetVoltage = voltageHighPower;
                // asic freq
                
                uint16_t freqNumber = freqHighPower;  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                currentPresetFrequency = freqHighPower;
    #elif (BitaxeUltra == 1 && BAPPORT == 1)
    Serial.println("High Power Mode Selected");
                // fan speed
                memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                delay(10);

                BAPFanSpeedBuffer[0] = 0x00;
                BAPFanSpeedBuffer[1] = 75; // 75%
                BAPAutoFanSpeedBuffer[0] = 0x00;
                BAPAutoFanSpeedBuffer[1] = autoFanModeHighPower; // Auto Fan Speed
                delay(10);
                currentPresetFanSpeed = 75;
                delay(10);
                currentPresetAutoFanMode = 0;
                delay(10);
                autoTuneEnabled = 1;
                //writeDataToBAP(BAPAutoFanSpeedBuffer, 2, BAP_AUTO_FAN_SPEED_BUFFER_REG);
                // asic voltage
                uint16_t voltageNumber = voltageHighPower;  // Convert string to number
                uint8_t voltageBytes[2] = {
                    (uint8_t)(voltageNumber >> 8),    // High byte
                    (uint8_t)(voltageNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                currentPresetVoltage = voltageHighPower;
                // asic freq
                
                uint16_t freqNumber = freqHighPower;  // Convert string to number
                uint8_t freqBytes[2] = {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                currentPresetFrequency = freqHighPower;

    #endif
}

void readCurrentPresetSettingsFromNVS() {

    currentPresetFrequency = loadSettingsFromNVSasU16(NVS_KEY_ASIC_CURRENT_FREQ);
    ESP_LOGI("ASIC", "Current Preset Frequency: %d", currentPresetFrequency);
    delay(10);
    currentPresetVoltage = loadSettingsFromNVSasU16(NVS_KEY_ASIC_CURRENT_VOLTAGE);
    delay(10);
    ESP_LOGI("ASIC", "Current Preset Voltage: %d", currentPresetVoltage);
    currentPresetFanSpeed = loadSettingsFromNVSasU16(NVS_KEY_ASIC_CURRENT_FAN_SPEED);
    delay(10);
    ESP_LOGI("ASIC", "Current Preset Fan Speed: %d", currentPresetFanSpeed);
    currentPresetAutoFanMode = loadSettingsFromNVSasU16(NVS_KEY_ASIC_CURRENT_AUTO_FAN_SPEED);
    delay(10);
    ESP_LOGI("ASIC", "Current Preset Auto Fan Mode: %d", currentPresetAutoFanMode);
    autoTuneEnabled = loadSettingsFromNVSasU16(NVS_KEY_ASIC_AUTOTUNE_ENABLED);
    delay(10);
    ESP_LOGI("ASIC", "Current Preset Auto Tune Enabled: %d", autoTuneEnabled);
}

bool isValidPresetPair(uint16_t freq, uint16_t voltage) {
    return ((freq == freqLowPower && voltage == voltageLowPower) ||
            (freq == freqNormalPower && voltage == voltageNormalPower) ||
            (freq == freqHighPower && voltage == voltageHighPower));
}


void presetAutoTune()
{

    // check that autotuning is enabled
    if(autoTuneEnabled == 0)
    {
        ESP_LOGI("ASIC", "Autotuning is disabled");
        return;
    }

    float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage;
    float power = IncomingData.monitoring.powerStats.power;
    float hashrate = IncomingData.mining.hashrate;
    uint32_t fanSpeedPercent = IncomingData.monitoring.fanSpeedPercent;
    uint32_t asicFreq = IncomingData.monitoring.asicFrequency;
    float asicTemp = IncomingData.monitoring.temperatures[0];
    uint16_t targetVoltage = IncomingData.monitoring.targetDomainVoltage;
    
    // Check that temp, freq, and voltage are within spec (This tells us the bitaxe is running)
    if (hashrate == 0 || asicFreq == 0 || domainVoltage == 0)
    {
        ESP_LOGE("Preset", "Missing Autotune Variable V: %.2f Freq: %lu Hashrate: %.2f", domainVoltage, asicFreq, hashrate);
        return;
    }
    ESP_LOGI("Preset", "Target V %.2u Target F %.2lu", targetVoltage, asicFreq);

    /*
    // check that the preset is set correctly in NVS compared to known preset values 
    if (isValidPresetPair(currentPresetFrequency, currentPresetVoltage) == false)
    {
         ESP_LOGE("Preset", "Preset does not match");
         return;
    }
    */

    
// check current temp to see if it is within good operating conditions
    if (asicTemp >= minPresetTemp && asicTemp <= maxPresetTemp )
    {
        ESP_LOGI("Preset", "Asic Temps good. Temp: %.2f", asicTemp);
        return;
    }
// set adjusted fan speed, voltage, and frequency based on current temps and power usage
    #if (BitaxeGamma == 1 || BitaxeUltra == 1)
    
    if(asicTemp >= maxPresetTemp)
    {
        // Try increasing fan speed first on lower settings
        ESP_LOGI("Preset", "Asic Temps hot. Tweaking Settings Temp: %.2f", asicTemp);
        if(fanSpeedPercent <= currentPresetFanSpeed + 5 && currentPresetAutoFanMode == 0)
        {   
            memset(BAPFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
            BAPFanSpeedBuffer[0] = 0x00;
            BAPFanSpeedBuffer[1] = fanSpeedPercent + 1;
            delay(10);
            writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
            ESP_LOGI("Preset", "Increasing Fan Speed to %lu", fanSpeedPercent + 1);
            return;
        }
        
        if(currentPresetFrequency * .9 <= asicFreq )
        {
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                uint16_t freqNumber = (asicFreq * 98) / 100;  // Integer math for 2% reduction
                uint8_t freqBytes[2] = 
                {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                delay(10);
                writeDataToBAP(BAPAsicFreqBuffer, 2 ,BAP_ASIC_FREQ_BUFFER_REG);
                ESP_LOGI("Preset", "Decreasing frequency to %u", freqNumber);
                
        }
        if (currentPresetVoltage *.95 <= targetVoltage)
        {
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                // Reduce voltage by 0.5% (multiply by 0.995)
                uint16_t newVoltage = (targetVoltage * 995) / 1000;  // Integer math to avoid floating point
                uint8_t voltageBytes[2] = {
                    (uint8_t)(newVoltage >> 8),    // High byte
                    (uint8_t)(newVoltage & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                delay(10);
                writeDataToBAP(BAPAsicVoltageBuffer, 2 ,BAP_ASIC_VOLTAGE_BUFFER_REG);
                ESP_LOGI("Preset", "Decreasing voltage to %u", newVoltage);

        } 
        else
        {
            ESP_LOGE("Preset", "nothing left to tweak. Change targets");
        }
    }
    if(asicTemp <= minPresetTemp)
      {
        // Try decreasing fan speed for noise first
        ESP_LOGI("Preset", "Asic Temp Overhead. Tweaking Settings Temp: %.2f", asicTemp);
        if(fanSpeedPercent >= currentPresetFanSpeed - 5 && currentPresetAutoFanMode == 0)
        {   
            memset(BAPFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
            BAPFanSpeedBuffer[0] = 0x00;
            BAPFanSpeedBuffer[1] = fanSpeedPercent - 1;
            delay(10);
            writeDataToBAP(BAPFanSpeedBuffer, 2, BAP_FAN_SPEED_BUFFER_REG);
            ESP_LOGI("Preset", "Decreasingg Fan Speed to %lu", fanSpeedPercent - 1);
            return;
        }
        
        if(currentPresetFrequency * 1.1 >= asicFreq )
        {
                memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
                uint16_t freqNumber = (asicFreq * 102) / 100;  // Integer math for 2% addition
                uint8_t freqBytes[2] = 
                {
                    (uint8_t)(freqNumber >> 8),    // High byte
                    (uint8_t)(freqNumber & 0xFF)   // Low byte
                };
                memcpy(BAPAsicFreqBuffer, freqBytes, 2);
                delay(10);
                writeDataToBAP(BAPAsicFreqBuffer, 2 ,BAP_ASIC_FREQ_BUFFER_REG);
                ESP_LOGI("Preset", "Increasing frequency to %u", freqNumber);
                
        }
        if (currentPresetVoltage *1.05 >= targetVoltage)
        {
                memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
                // Reduce voltage by 0.5% (multiply by 0.995)
                uint16_t newVoltage = (targetVoltage * 1005) / 1000;  // Integer math to avoid floating point
                uint8_t voltageBytes[2] = {
                    (uint8_t)(newVoltage >> 8),    // High byte
                    (uint8_t)(newVoltage & 0xFF)   // Low byte
                };
                memcpy(BAPAsicVoltageBuffer, voltageBytes, 2);
                delay(10);
                writeDataToBAP(BAPAsicVoltageBuffer, 2 ,BAP_ASIC_VOLTAGE_BUFFER_REG);
                ESP_LOGI("Preset", "Increasing voltage to %u", newVoltage);

        } 
        else
        {
            ESP_LOGE("Preset", "nothing left to tweak. Change targets");
        }
    }

    #endif
}
