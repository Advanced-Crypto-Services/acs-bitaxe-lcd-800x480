#pragma once

#include <Arduino.h>
#include <esp_attr.h>
#include "UIScreens.h"
#include <lvgl.h>
#include <TimeLib.h>

#include "modelConfig.h"



#define PSRAM_ATTR __attribute__((section(".psram")))

#define BAP_TX 16
#define BAP_RX 15


// Special Registers
// Special registers (0xF0 - 0xFF)
#define LVGL_REG_SPECIAL_THEME 0xF0
#define LVGL_REG_SPECIAL_PRESET 0xF1
#define LVGL_REG_SPECIAL_RESTART 0xFE
// Flags (0xE0 - 0xEF)
#define LVGL_FLAG_STARTUP_DONE 0xE0
#define LVGL_FLAG_OVERHEAT_MODE 0xE1
#define LVGL_FLAG_FOUND_BLOCK 0xE2

struct SpecialRegisters
{
    uint8_t restart;
    uint8_t startupDone;
    uint8_t overheatMode;
    uint8_t foundBlock;
};

extern SpecialRegisters specialRegisters;

#define BAPReadBufferLength 512
extern uint8_t* BAPReadBuffer;
#define BAPWriteBufferLength 512
extern uint8_t* BAPWriteBuffer;
// Define the registers for the BAP

#define LVGL_REG_SSID           0x21
#define LVGL_REG_IP_ADDR        0x22
#define LVGL_REG_WIFI_STATUS    0x23
#define LVGL_REG_POOL_URL       0x24
#define LVGL_REG_FALLBACK_URL   0x25
#define LVGL_REG_POOL_PORTS     0x26

// Mining data registers (5 second updates)
#define LVGL_REG_HASHRATE        0x30
#define LVGL_REG_HIST_HASHRATE   0x31
#define LVGL_REG_EFFICIENCY      0x32
#define LVGL_REG_BEST_DIFF       0x33
#define LVGL_REG_SESSION_DIFF    0x34
#define LVGL_REG_SHARES          0x35

// Monitoring data registers (5 second updates)
#define LVGL_REG_TEMPS           0x40 // 8 * float
#define LVGL_REG_ASIC_FREQ      0x41 // float
#define LVGL_REG_FAN            0x42 // 2 * float
#define LVGL_REG_POWER_STATS    0x43 // 4 * float
#define LVGL_REG_ASIC_INFO      0x44 // uint16
#define LVGL_REG_UPTIME         0x45 // uint32 Look into moving to 64 bit to increase time 
#define LVGL_REG_VREG_TEMP      0x46 // float

// Device status registers (on change only)
#define LVGL_REG_FLAGS          0x50 // 4 bytes

#define LVGL_REG_DEVICE_INFO   0x52 // not used
#define LVGL_REG_BOARD_INFO    0x53 // variable length
#define LVGL_REG_CLOCK_SYNC    0x54 // uint32

// API Data Registers
#define LVGL_REG_API_BTC_PRICE   0x60 // uint32
#define LVGL_REG_API_NETWORK_HASHRATE 0x61 // double
#define LVGL_REG_API_NETWORK_DIFFICULTY 0x62 // double
#define LVGL_REG_API_BLOCK_HEIGHT 0x63 // uint32
#define LVGL_REG_API_DIFFICULTY_PROGRESS 0x64 // double
#define LVGL_REG_API_DIFFICULTY_CHANGE 0x65 // double
#define LVGL_REG_API_REMAINING_BLOCKS 0x66 // uint32
#define LVGL_REG_API_REMAINING_TIME 0x67 // uint32
#define LVGL_REG_API_FASTEST_FEE 0x68 // uint32
#define LVGL_REG_API_HALF_HOUR_FEE 0x69 // uint32
#define LVGL_REG_API_HOUR_FEE 0x6A // uint32
#define LVGL_REG_API_ECONOMY_FEE 0x6B // uint32
#define LVGL_REG_API_MINIMUM_FEE 0x6C // uint32

// device info registers
// define max lengths for serial number, model, and firmware version
#define MAX_SERIAL_LENGTH 32
#define MAX_MODEL_LENGTH 32
#define MAX_FIRMWARE_VERSION_LENGTH 32
#define MAX_THEME_LENGTH 128
#define LVGL_REG_DEVICE_SERIAL 0x70 // MAX_SERIAL_LENGTH bytes
#define LVGL_REG_BOARD_MODEL 0x71 // MAX_MODEL_LENGTH bytes
#define LVGL_REG_BOARD_FIRMWARE_VERSION 0x72 // MAX_FIRMWARE_VERSION_LENGTH bytes
#define LVGL_REG_THEME_CURRENT 0x73 // 128 bytes
#define LVGL_REG_THEMES_AVAILABLE 0x74 // 128 bytes

// Settings registers
#define LVGL_REG_SETTINGS_HOSTNAME 0xA0 // 32 bytes
#define LVGL_REG_SETTINGS_WIFI_SSID 0xA1 // 32 bytes
#define LVGL_REG_SETTINGS_WIFI_PASSWORD 0xA2 // 32 bytes

// Maximum lengths for strings
#define MAX_SSID_LENGTH       32
#define MAX_IP_LENGTH        16
#define MAX_URL_LENGTH       128
#define MAX_STATUS_LENGTH    32
#define MAX_INFO_LENGTH      64
#define MAX_DIFF_LENGTH      16
#define MAX_THEME_LENGTH     32
#define MAX_PRESET_LENGTH    32

// Maximum lengths for numeric data
#define MAX_FLOAT_SIZE      sizeof(float)
#define MAX_UINT32_SIZE    sizeof(uint32_t)
#define MAX_UINT64_SIZE    sizeof(uint64_t)
#define MAX_UINT16_SIZE    sizeof(uint16_t)
#define MAX_TEMPS_COUNT    8

// Define separate buffers for settings
// Network settings
// Hostname
#define BAP_HOSTNAME_BUFFER_REG 0x90
#define BAP_HOSTNAME_BUFFER_SIZE 32
extern uint8_t* BAPHostnameBuffer;
// WiFi SSID
#define BAP_WIFI_SSID_BUFFER_REG 0x91
#define BAP_WIFI_SSID_BUFFER_SIZE 64
extern uint8_t* BAPWifiSSIDBuffer;
// WiFi Password
#define BAP_WIFI_PASS_BUFFER_REG 0x92
#define BAP_WIFI_PASS_BUFFER_SIZE 64
extern uint8_t* BAPWifiPassBuffer;
// Stratum URL Main
#define BAP_STRATUM_URL_MAIN_BUFFER_REG 0x93
#define BAP_STRATUM_URL_MAIN_BUFFER_SIZE 128
extern uint8_t* BAPStratumUrlMainBuffer;
// Stratum Port Main
#define BAP_STRATUM_PORT_MAIN_BUFFER_REG 0x94
#define BAP_STRATUM_PORT_MAIN_BUFFER_SIZE 2
extern uint8_t* BAPStratumPortMainBuffer;
// Stratum User Main
#define BAP_STRATUM_USER_MAIN_BUFFER_REG 0x95
#define BAP_STRATUM_USER_MAIN_BUFFER_SIZE 128
extern uint8_t* BAPStratumUserMainBuffer;  
// Stratum Password Main
#define BAP_STRATUM_PASS_MAIN_BUFFER_REG 0x96
#define BAP_STRATUM_PASS_MAIN_BUFFER_SIZE 64
extern uint8_t* BAPStratumPassMainBuffer;
// Stratum URL Fallback
#define BAP_STRATUM_URL_FALLBACK_BUFFER_REG 0x97
#define BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE 128
extern uint8_t* BAPStratumUrlFallbackBuffer;
// Stratum Port Fallback
#define BAP_STRATUM_PORT_FALLBACK_BUFFER_REG 0x98
#define BAP_STRATUM_PORT_FALLBACK_BUFFER_SIZE 2
extern uint8_t* BAPStratumPortFallbackBuffer;
// Stratum User Fallback
#define BAP_STRATUM_USER_FALLBACK_BUFFER_REG 0x99
#define BAP_STRATUM_USER_FALLBACK_BUFFER_SIZE 128
extern uint8_t* BAPStratumUserFallbackBuffer;
// Stratum Password Fallback
#define BAP_STRATUM_PASS_FALLBACK_BUFFER_REG 0x9A
#define BAP_STRATUM_PASS_FALLBACK_BUFFER_SIZE 64
extern uint8_t* BAPStratumPassFallbackBuffer;
// ASIC Voltage
#define BAP_ASIC_VOLTAGE_BUFFER_REG 0x9B
#define BAP_ASIC_VOLTAGE_BUFFER_SIZE 2
extern uint8_t* BAPAsicVoltageBuffer;
// Asic Freq
#define BAP_ASIC_FREQ_BUFFER_REG 0x9C
#define BAP_ASIC_FREQ_BUFFER_SIZE 2
extern uint8_t* BAPAsicFreqBuffer;
// Fan Speed
#define BAP_FAN_SPEED_BUFFER_REG 0x9D
#define BAP_FAN_SPEED_BUFFER_SIZE 2
extern uint8_t* BAPFanSpeedBuffer;
// Auto Fan Speed
#define BAP_AUTO_FAN_SPEED_BUFFER_REG 0x9E
#define BAP_AUTO_FAN_SPEED_BUFFER_SIZE 2
extern uint8_t* BAPAutoFanSpeedBuffer;

// Define the data structures for the BAP

struct NetworkData 
{
    char ssid[MAX_SSID_LENGTH];
    char ipAddress[MAX_IP_LENGTH];
    char wifiStatus[MAX_STATUS_LENGTH];
    char poolUrl[MAX_URL_LENGTH];
    char fallbackUrl[MAX_URL_LENGTH];
    uint16_t poolPort;
    uint16_t fallbackPort;
};

// Mining data structure
struct MiningData 
{
    float hashrate;
    float historicalHashrate;
    float efficiency;
    char bestDiff[MAX_DIFF_LENGTH];
    char sessionDiff[MAX_DIFF_LENGTH];
    uint32_t shares;
    int acceptedShares;
    int rejectedShares;
    float rejectRatePercent;
};

// Monitoring data structure
struct MonitoringData 
{
    float temperatures[8];    // Array for multiple temperature sensors For Hex model
    uint32_t asicFrequency;
    float fanSpeed;
    float fanSpeedPercent;
    struct 
    {
        float voltage;
        float current;
        float power;
        float domainVoltage;
    } powerStats;
    uint32_t asicInfo;
    uint32_t uptime;
    uint16_t targetDomainVoltage;
    float vregTemp;
};

// Device status structure
struct DeviceStatus 
{
    uint32_t flags;
    char deviceInfo[MAX_INFO_LENGTH];
    char boardInfo[MAX_INFO_LENGTH];
    uint32_t clockSync;
    char theme[MAX_THEME_LENGTH];
    char preset[MAX_PRESET_LENGTH];
    char serialNumber[MAX_SERIAL_LENGTH];
    char chipModel[MAX_MODEL_LENGTH];
    char firmwareVersion[MAX_FIRMWARE_VERSION_LENGTH];
};

// API Data Structure
struct APIData
{
    uint32_t btcPriceUSD;
    double networkHashrate;
    double networkDifficulty;
    uint32_t blockHeight;
    double difficultyProgressPercent;
    double difficultyChangePercent;
    uint32_t remainingBlocksToDifficultyAdjustment;
    uint32_t remainingTimeToDifficultyAdjustment;
    uint32_t fastestFee;
    uint32_t halfHourFee;
    uint32_t hourFee;
    uint32_t economyFee;
    uint32_t minimumFee;
};

// Global data container
struct IncomingDataContainer 
{
    NetworkData network;
    MiningData mining;
    MonitoringData monitoring;
    DeviceStatus status;
    APIData api;
};

extern bool confirmedOverheatMode;
extern bool confirmedFoundBlock;

// External declarations

extern PSRAM_ATTR IncomingDataContainer IncomingData;

extern void initializeBAPBuffers();
extern void setupBAP();
extern void readDataFromBAP();
extern void writeDataToBAP(uint8_t* buffer, size_t len, uint8_t reg);
extern void handleAPIDataSerial(uint8_t* buffer, uint8_t len);
extern void handleNetworkDataSerial(uint8_t* buffer, uint8_t len);
extern void handleMiningDataSerial(uint8_t* buffer, uint8_t len);
extern void handleMonitoringDataSerial(uint8_t* buffer, uint8_t len);
extern void handleDeviceStatusSerial(uint8_t* buffer, uint8_t len);
extern void handleFlagsDataSerial(uint8_t* buffer, uint8_t len);
extern void handleSpecialRegistersSerial(uint8_t* buffer, uint8_t len);
extern void sendRestartToBAP();

