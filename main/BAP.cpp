#include "BAP.h"
#include "modelConfig.h"
#include "UIScreens.h"
#include "modelPresets.h"
#include "NVS.h"

uint8_t* BAPReadBuffer = nullptr;
uint8_t* BAPWriteBuffer = nullptr;
uint8_t* BAPHostnameBuffer = nullptr;
uint8_t* BAPWifiSSIDBuffer = nullptr;
uint8_t* BAPWifiPassBuffer = nullptr;
uint8_t* BAPStratumUrlMainBuffer = nullptr;
uint8_t* BAPStratumPortMainBuffer = nullptr;
uint8_t* BAPStratumUserMainBuffer = nullptr;
uint8_t* BAPStratumPassMainBuffer = nullptr;
uint8_t* BAPStratumUrlFallbackBuffer = nullptr;
uint8_t* BAPStratumPortFallbackBuffer = nullptr;
uint8_t* BAPStratumUserFallbackBuffer = nullptr;
uint8_t* BAPStratumPassFallbackBuffer = nullptr;
uint8_t* BAPAsicVoltageBuffer = nullptr;
uint8_t* BAPAsicFreqBuffer = nullptr;
uint8_t* BAPFanSpeedBuffer = nullptr;
uint8_t* BAPAutoFanSpeedBuffer = nullptr;

SpecialRegisters specialRegisters = {0};
IncomingDataContainer PSRAM_ATTR IncomingData = {0};

#define FLAG_CONFIRMATION_COUNT 3 
uint8_t overheatModeCounter = 0;
uint8_t foundBlockCounter = 0;
bool confirmedOverheatMode = false;
bool confirmedFoundBlock = false;

#define __min(a,b) ((a)<(b)?(a):(b))

// CRC Buffers
static uint8_t CRCTXBuffer[2];
static uint8_t CRCRXBuffer[2];

/// @brief waits for a serial response to Match CRC
/// @param expectedCRC the expected crc 
/// @param timeout_ms number of ms to wait before timing out
/// @return true if CRC matches, false otherwise
static bool CRCRxCheck(uint16_t expectedCRC, uint16_t timeout_ms)
{
    // Clear any pending data in the read buffer first
    while (Serial2.available()) {
        Serial2.read();
    }

    memset(CRCRXBuffer, 0, 2);
    uint32_t startTime = millis();
    uint8_t bytesRead = 0;
    
    // Read exactly 2 bytes for the CRC response
    while (bytesRead < 2 && (millis() - startTime < timeout_ms)) {
        if (Serial2.available() > 0) {
            CRCRXBuffer[bytesRead] = Serial2.read();
            ESP_LOGI("BAP", "Read CRC byte %d: 0x%02X", bytesRead, CRCRXBuffer[bytesRead]);
            bytesRead++;
        }
        yield();
        delay(5);  // Add a small delay to give more time between reads
    }
    
    // Verify we got exactly 2 bytes
    if (bytesRead != 2) {
        ESP_LOGE("BAP", "Failed to read CRC from device after %d ms. Got %d bytes", timeout_ms, bytesRead);
        return false;
    }
    
    // Construct the received CRC (high byte first)
    uint16_t receivedCRC = (CRCRXBuffer[0] << 8) | CRCRXBuffer[1];
    
    // Log both CRCs for comparison
    ESP_LOGI("BAP", "CRC check - Expected: 0x%04X, Received: 0x%04X", expectedCRC, receivedCRC);
    
    if (receivedCRC == expectedCRC) {
        ESP_LOGI("BAP", "CRC match confirmed");
        return true;
    }
    
    ESP_LOGE("BAP", "CRC mismatch: expected 0x%04X, received 0x%04X", expectedCRC, receivedCRC);
    return false;
}

/// @brief Calculates the CRC16 of a given data
/// @param data The data to calculate the CRC16 of
/// @param length The length of the data
/// @return The CRC16 of the data
static uint16_t calculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;  // Initial value
    
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    
    return crc;
}

void initializeBAPBuffers()
{
    BAPReadBuffer = (uint8_t*)malloc(BAPReadBufferLength);
    BAPWriteBuffer = (uint8_t*)malloc(BAPWriteBufferLength);
    BAPHostnameBuffer = (uint8_t*)malloc(BAP_HOSTNAME_BUFFER_SIZE);
    BAPWifiSSIDBuffer = (uint8_t*)malloc(BAP_WIFI_SSID_BUFFER_SIZE);
    BAPWifiPassBuffer = (uint8_t*)malloc(BAP_WIFI_PASS_BUFFER_SIZE);
    BAPStratumUrlMainBuffer = (uint8_t*)malloc(BAP_STRATUM_URL_MAIN_BUFFER_SIZE);
    BAPStratumPortMainBuffer = (uint8_t*)malloc(BAP_STRATUM_PORT_MAIN_BUFFER_SIZE);
    BAPStratumUserMainBuffer = (uint8_t*)malloc(BAP_STRATUM_USER_MAIN_BUFFER_SIZE);
    BAPStratumPassMainBuffer = (uint8_t*)malloc(BAP_STRATUM_PASS_MAIN_BUFFER_SIZE);
    BAPStratumUrlFallbackBuffer = (uint8_t*)malloc(BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE);
    BAPStratumPortFallbackBuffer = (uint8_t*)malloc(BAP_STRATUM_PORT_FALLBACK_BUFFER_SIZE);
    BAPStratumUserFallbackBuffer = (uint8_t*)malloc(BAP_STRATUM_USER_FALLBACK_BUFFER_SIZE);
    BAPStratumPassFallbackBuffer = (uint8_t*)malloc(BAP_STRATUM_PASS_FALLBACK_BUFFER_SIZE);
    BAPAsicVoltageBuffer = (uint8_t*)malloc(BAP_ASIC_VOLTAGE_BUFFER_SIZE);
    BAPAsicFreqBuffer = (uint8_t*)malloc(BAP_ASIC_FREQ_BUFFER_SIZE);
    BAPFanSpeedBuffer = (uint8_t*)malloc(BAP_FAN_SPEED_BUFFER_SIZE);
    BAPAutoFanSpeedBuffer = (uint8_t*)malloc(BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
    memset(BAPFanSpeedBuffer, 0, BAP_FAN_SPEED_BUFFER_SIZE);
    memset(BAPAutoFanSpeedBuffer, 0, BAP_AUTO_FAN_SPEED_BUFFER_SIZE);
    memset(BAPAsicVoltageBuffer, 0, BAP_ASIC_VOLTAGE_BUFFER_SIZE);
    memset(BAPAsicFreqBuffer, 0, BAP_ASIC_FREQ_BUFFER_SIZE);
}

void setupBAP()
{
    Serial2.begin(115200, SERIAL_8N1, BAP_RX, BAP_TX);
    uint32_t startTime = millis();
    while (!Serial2)
    {
        if (millis() - startTime > 5000) {  // 5 second timeout
            ESP_LOGE("BAP", "Failed to initialize Serial2");
            return;
        }
        yield();
    }
    ESP_LOGI("BAP", "BAP Setup Complete - RX: %d, TX: %d", BAP_RX, BAP_TX);
}

void processMessage(uint8_t* message, uint8_t dataLen, uint8_t reg) {
    // Group registers into ranges for switch cases
    switch(reg) {
        // Network Data (0x21-0x26)
        case 0x21 ... 0x26:
            handleNetworkDataSerial(message, dataLen);
            break;
            
        // Mining Data (0x30-0x35)
        case 0x30 ... 0x35:
            handleMiningDataSerial(message, dataLen);
            break;
            
        // Monitoring Data (0x40-0x46)
        case 0x40 ... 0x46:
            handleMonitoringDataSerial(message, dataLen);
            break;
            
        // Device Status (0x50-0x54)
        case 0x50 ... 0x54:
            handleDeviceStatusSerial(message, dataLen);
            break;
            
        // API Data (0x60-0x6C)
        case 0x60 ... 0x6C:
            handleAPIDataSerial(message, dataLen);
            break;

        // Device Info Data (0x70-0x74)
        case 0x70 ... 0x74:
            handleDeviceStatusSerial(message, dataLen);
            break;
            
        // Flags Data (0xE0-0xEF)
        case 0xE0 ... 0xEF:
            handleFlagsDataSerial(message, dataLen);
            break;
            
        // Special Registers (0xF0-0xFF)
        case 0xF0 ... 0xFF:
            handleSpecialRegistersSerial(message, dataLen);
            break;
            
        default:
            ESP_LOGW("BAP", "Unknown register 0x%02X", reg);
            break;
    }
}


void readDataFromBAP() {
    // Check if buffer is initialized
    if (BAPReadBuffer == nullptr) {
        ESP_LOGE("BAP", "BAPReadBuffer not initialized");
        return;
    }
    
    // Check if there's actually data available in Serial2's buffer
    if (!Serial2.available()) {
        return;
    }

    // Read all available data from Serial2's buffer
    int16_t bytes_read = Serial2.readBytes(BAPReadBuffer, BAPReadBufferLength);
    ESP_LOGI("BAP", "Read %d bytes from Serial2 buffer", bytes_read);
    
    // Log all bytes in one row
    char log_buffer[1024];  // Adjust size as needed
    int offset = 0;
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "BAPReadBuffer: ");
    for (int i = 0; i < bytes_read; i++) {
        offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "POS:%d %02X ", i, BAPReadBuffer[i]);
    }
    ESP_LOGI("BAP", "%s", log_buffer);

    // Process messages one by one
    size_t current_pos = 0;
    while (current_pos < bytes_read - 6) {
        if (BAPReadBuffer[current_pos] == 0xFF && BAPReadBuffer[current_pos + 1] == 0xAA) {
            uint8_t reg = BAPReadBuffer[current_pos + 2];
            uint8_t dataLen = BAPReadBuffer[current_pos + 3];
            
            if (current_pos + 6 + dataLen <= bytes_read) {
                
                // Calculate CRC of received message
                uint16_t received_crc = (BAPReadBuffer[current_pos + 4 + dataLen] << 8) | 
                                      BAPReadBuffer[current_pos + 5 + dataLen];
                
                uint16_t calculated_crc = calculateCRC16(&BAPReadBuffer[current_pos + 2], dataLen + 2);
                
                ESP_LOGI("BAP", "Received CRC: 0x%04X, Calculated CRC: 0x%04X", 
                         received_crc, calculated_crc);

                if (received_crc == calculated_crc) {
                    processMessage(&BAPReadBuffer[current_pos], dataLen, reg);
                } else {
                    ESP_LOGE("BAP", "CRC Mismatch in message at position %d", current_pos);
                }
                
                current_pos += 6 + dataLen;
            } else {
                // Incomplete message, wait for next read
                break;
            }
        } else {
            current_pos++;
        }
    }
}

void writeDataToBAP(uint8_t* buffer, size_t dataLen, uint8_t reg) {
    // Calculate total size needed: preamble(2) + reg(1) + len(1) + data + CRC(2)
    const size_t totalSize = 4 + dataLen + 2;
    const int maxRetries = 3;  // Maximum number of retry attempts
    int retryCount = 0;
    
    while (retryCount < maxRetries) {
        // Fill the write buffer
        size_t offset = 0;
        BAPWriteBuffer[0] = 0xFF;  // Preamble
        BAPWriteBuffer[1] = 0xAA;  // Preamble
        BAPWriteBuffer[2] = reg;   // Register
        BAPWriteBuffer[3] = dataLen; // Length
        
        // Copy data
        memcpy(&BAPWriteBuffer[4], buffer, dataLen);

        // Calculate and append CRC - now only over reg + len + data
        uint16_t calculatedCRC = calculateCRC16(&BAPWriteBuffer[2], dataLen + 2);
        BAPWriteBuffer[dataLen + 4] = (calculatedCRC >> 8) & 0xFF;  // High byte
        BAPWriteBuffer[dataLen + 5] = calculatedCRC & 0xFF;         // Low byte
        ESP_LOGI("BAP", "CRC: 0x%04X", calculatedCRC);
        ESP_LOGI("BAP", "BAPWriteBuffer: ");
        for (int i = 0; i < totalSize; i++)
        {
            ESP_LOGI("BAP", "%02X ", BAPWriteBuffer[i]);
        }
        Serial0.printf("\n");

        // Send everything in one write
        Serial2.write(BAPWriteBuffer, totalSize);
        // Wait for CRC Feedback
        if (CRCRxCheck(calculatedCRC, 1000)) {
            ESP_LOGI("BAP", "CRC Matched");
            return;  // Success, exit function
        }
        
        // If we get here, either CRC didn't match or we didn't get a response
        retryCount++;
        ESP_LOGE("BAP", "No valid CRC response, attempt %d of %d", retryCount, maxRetries);
        if (retryCount < maxRetries) {
            delay(500);  // Wait a bit before retrying
            continue;    // Explicitly continue to next retry
        }
    }
    
    ESP_LOGE("BAP", "Failed to send data after %d attempts", maxRetries);
}

void handleNetworkDataSerial(uint8_t* buffer, uint8_t len)
{
    if (len < 2) return;
    
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];
    
    switch(reg) 
    {
        case LVGL_REG_SSID:
            memset(IncomingData.network.ssid, 0, MAX_SSID_LENGTH);
            memcpy(IncomingData.network.ssid, &buffer[4], __min(dataLen, MAX_SSID_LENGTH - 1));
            break;
        case LVGL_REG_IP_ADDR:
            memset(IncomingData.network.ipAddress, 0, MAX_IP_LENGTH);
            memcpy(IncomingData.network.ipAddress, &buffer[4], __min(dataLen, MAX_IP_LENGTH - 1));
            break;
        case LVGL_REG_WIFI_STATUS:
            memset(IncomingData.network.wifiStatus, 0, MAX_STATUS_LENGTH);
            memcpy(IncomingData.network.wifiStatus, &buffer[4], __min(dataLen, MAX_STATUS_LENGTH - 1));
            break;
        case LVGL_REG_POOL_URL:
            memset(IncomingData.network.poolUrl, 0, MAX_URL_LENGTH);
            memcpy(IncomingData.network.poolUrl, &buffer[4], __min(dataLen, MAX_URL_LENGTH - 1));
            break;
        case LVGL_REG_FALLBACK_URL:
            memset(IncomingData.network.fallbackUrl, 0, MAX_URL_LENGTH);
            memcpy(IncomingData.network.fallbackUrl, &buffer[4], __min(dataLen, MAX_URL_LENGTH - 1));
            break;
        case LVGL_REG_POOL_PORTS:
                if (dataLen >= 4) 
            {
                IncomingData.network.poolPort = (buffer[5] << 8) | buffer[4];
                IncomingData.network.fallbackPort = (buffer[7] << 8) | buffer[6];
                Serial.printf("Pool: %s : %d, Fallback: %s : %d\n", IncomingData.network.poolUrl, IncomingData.network.poolPort, IncomingData.network.fallbackUrl, IncomingData.network.fallbackPort);
            }
            break;
        default:
            Serial.printf("Warning: Unknown network register 0x%02X with length %d\n", reg, dataLen);
            break;
    }

}

void handleMonitoringDataSerial(uint8_t* buffer, uint8_t len) 
{
    if (len < 2) return;
    
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];
    
    switch(reg) {
        case LVGL_REG_TEMPS: 
        {
            // Create union to safely handle the byte-to-float conversion
            union {
                uint8_t bytes[4];
                float value;
            } temp;

            // Read bytes in correct order (assuming little-endian)
            temp.bytes[0] = buffer[8];
            temp.bytes[1] = buffer[9];
            temp.bytes[2] = buffer[10];
            temp.bytes[3] = buffer[11];

            IncomingData.monitoring.temperatures[0] = temp.value;
            Serial.printf("Temperature received: %.2f°C\n", IncomingData.monitoring.temperatures[0]);
            break;
        }
        case LVGL_REG_ASIC_FREQ:
        {
            float asicFreq;
            memset(&asicFreq, 0, sizeof(float));
            memcpy(&asicFreq, &buffer[4], sizeof(float));
            IncomingData.monitoring.asicFrequency = asicFreq;
            Serial.print("ASIC Frequency received: ");
            Serial.printf("%lu\n", IncomingData.monitoring.asicFrequency);
            break;
        }
        case LVGL_REG_FAN:
        {
            float fanStats[2];  // Array to hold RPM and percentage
            memset(fanStats, 0, sizeof(float) * 2);
            memcpy(fanStats, &buffer[4], __min(dataLen, sizeof(float) * 2));
            IncomingData.monitoring.fanSpeed = fanStats[0];
            IncomingData.monitoring.fanSpeedPercent = fanStats[1];
            
            Serial.printf("Fan RPM: %f, Fan Percent: %f%%\n", 
                IncomingData.monitoring.fanSpeed,
                IncomingData.monitoring.fanSpeedPercent);
            break;
        }
        case LVGL_REG_POWER_STATS:
        {
            float powerStatsArray[4];
            memset(powerStatsArray, 0, sizeof(float) * 4);
            memcpy(powerStatsArray, &buffer[4], __min(dataLen, sizeof(float) * 4));
            IncomingData.monitoring.powerStats.voltage = powerStatsArray[0];
            IncomingData.monitoring.powerStats.current = powerStatsArray[1];
            IncomingData.monitoring.powerStats.power = powerStatsArray[2];
            IncomingData.monitoring.powerStats.domainVoltage = powerStatsArray[3];
            Serial.printf("Voltage: %f, Current: %f, Power: %f, Domain Voltage: %f\n", 
                IncomingData.monitoring.powerStats.voltage,
                IncomingData.monitoring.powerStats.current,
                IncomingData.monitoring.powerStats.power,
                IncomingData.monitoring.powerStats.domainVoltage);
            break;
        }
        case LVGL_REG_ASIC_INFO:
        {
            memset(&IncomingData.monitoring.asicInfo, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.monitoring.asicInfo, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            break;
        }
        case LVGL_REG_UPTIME:
        {
            memset(&IncomingData.monitoring.uptime, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.monitoring.uptime, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            break;  
        }
        case LVGL_REG_VREG_TEMP:
        {
            float vregTemp;
            memset(&vregTemp, 0, sizeof(float));
            memcpy(&vregTemp, &buffer[4], sizeof(float));
            IncomingData.monitoring.vregTemp = vregTemp;
            ESP_LOGI("BAP", "VREG Temperature received: %.2f°C", IncomingData.monitoring.vregTemp);
            break;
        }
        default:
        {   
            Serial.printf("Warning: Unknown monitoring register 0x%02X with length %d\n", reg, dataLen);
            break;
        }
    }
}

void handleDeviceStatusSerial(uint8_t* buffer, uint8_t len) 
{
    // if (len < 2) return;
    
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];
    
    switch(reg)
    {
        case LVGL_REG_DEVICE_SERIAL:
        {
            memset(IncomingData.status.serialNumber, 0, MAX_SERIAL_LENGTH);
            memcpy(IncomingData.status.serialNumber, &buffer[4], __min(dataLen, MAX_SERIAL_LENGTH - 1));
            ESP_LOGI("BAP", "Serial Number received: %s", IncomingData.status.serialNumber);
            break;
        }
        case LVGL_REG_BOARD_MODEL:
        {
            memset(IncomingData.status.chipModel, 0, MAX_MODEL_LENGTH);
            memcpy(IncomingData.status.chipModel, &buffer[4], __min(dataLen, MAX_MODEL_LENGTH - 1));
            ESP_LOGI("BAP", "Chip Model received: %s", IncomingData.status.chipModel);
            break;
        }
        case LVGL_REG_BOARD_FIRMWARE_VERSION:
        {
            memset(IncomingData.status.firmwareVersion, 0, MAX_FIRMWARE_VERSION_LENGTH);
            memcpy(IncomingData.status.firmwareVersion, &buffer[4], __min(dataLen, MAX_FIRMWARE_VERSION_LENGTH - 1));
            ESP_LOGI("BAP", "Firmware Version received: %s", IncomingData.status.firmwareVersion);
            break;
        }
        default:
        {
            Serial.printf("Warning: Unknown device status register 0x%02X with length %d\n", reg, dataLen);
            break;
        }
    }
}

void handleMiningDataSerial(uint8_t* buffer, uint8_t len) 
{
    // must be more than the register and length bytes
    if (len < 2) return;
    
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];
    
    
    switch(reg) 
    {
        case LVGL_REG_HASHRATE: 
        {
            float hashrate;
            memset(&hashrate, 0, sizeof(float));
            memcpy(&hashrate, &buffer[4], sizeof(float));
            IncomingData.mining.hashrate = hashrate;
            Serial.print("Hashrate received: ");
            Serial.println(IncomingData.mining.hashrate);
            break;
        }
        case LVGL_REG_HIST_HASHRATE: 
        {
            float historicalHashrate;
            memset(&historicalHashrate, 0, sizeof(float));
            memcpy(&historicalHashrate, &buffer[4], sizeof(float));
            IncomingData.mining.historicalHashrate = historicalHashrate;
            Serial.print("Historical Hashrate received: ");
            Serial.println(IncomingData.mining.historicalHashrate);
            break;
        }
        case LVGL_REG_EFFICIENCY: 
        {
            float efficiency;
            memset(&efficiency, 0, sizeof(float));
            memcpy(&efficiency, &buffer[4], sizeof(float));
            IncomingData.mining.efficiency = efficiency;
            Serial.print("Efficiency received: ");
            Serial.println(IncomingData.mining.efficiency);
            break;
        }
        case LVGL_REG_BEST_DIFF:
        {
            memset(IncomingData.mining.bestDiff, 0, MAX_DIFF_LENGTH);
            memcpy(IncomingData.mining.bestDiff, &buffer[4], __min(dataLen, MAX_DIFF_LENGTH - 1));
            IncomingData.mining.bestDiff[dataLen] = '\0';  // Null terminate the string
            Serial.print("Best Diff received: ");
            Serial.println(IncomingData.mining.bestDiff);
            break;
        }
        case LVGL_REG_SESSION_DIFF:
        {
            memset(IncomingData.mining.sessionDiff, 0, MAX_DIFF_LENGTH);
            memcpy(IncomingData.mining.sessionDiff, &buffer[4], __min(dataLen, MAX_DIFF_LENGTH - 1));
            IncomingData.mining.sessionDiff[dataLen] = '\0';  // Null terminate the string
            Serial.print("Session Diff received: ");
            Serial.println(IncomingData.mining.sessionDiff);
            break;
        }
        case LVGL_REG_SHARES:
        // Each share is 4 bytes, sent in the same I2C sequence. Accepted Shares first, then Rejected Shares
        {
            memset(&IncomingData.mining.acceptedShares, 0, sizeof(uint32_t));
            memset(&IncomingData.mining.rejectedShares, 0, sizeof(uint32_t));
            IncomingData.mining.acceptedShares = (buffer[4] << 0) | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
            IncomingData.mining.rejectedShares = (buffer[8] << 0) | (buffer[9] << 8) | (buffer[10] << 16) | (buffer[11] << 24);
            IncomingData.mining.shares = IncomingData.mining.acceptedShares + IncomingData.mining.rejectedShares;
            Serial.printf("Accepted shares: %d, Rejected shares: %d, Total shares: %lu\n", 
                IncomingData.mining.acceptedShares, 
                IncomingData.mining.rejectedShares,
                IncomingData.mining.shares);
            if (IncomingData.mining.shares > 0) 
            {
                IncomingData.mining.rejectRatePercent = (IncomingData.mining.rejectedShares * 100.0f) / IncomingData.mining.shares;
            } 
            else 
            {
                IncomingData.mining.rejectRatePercent = 0.0f;
            }
            break;
        }
        default:
            Serial.printf("Warning: Unknown mining register 0x%02X with length %d\n", reg, dataLen);
            break;
    }
}

void handleAPIDataSerial(uint8_t* buffer, uint8_t len) 
{
        if (len < 2) return;
    
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];

    switch(reg)
    {
        case LVGL_REG_API_BTC_PRICE:
        {
            memset(&IncomingData.api.btcPriceUSD, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.btcPriceUSD, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("BTC Price received: %lu\n", IncomingData.api.btcPriceUSD);
            break;
        }
        case LVGL_REG_API_NETWORK_HASHRATE:
        {
            memset(&IncomingData.api.networkHashrate, 0, sizeof(double));
            memcpy(&IncomingData.api.networkHashrate, &buffer[4], __min(dataLen, sizeof(double)));
            Serial.printf("Network Hashrate received: %f\n", IncomingData.api.networkHashrate);
            break;
        }
        case LVGL_REG_API_NETWORK_DIFFICULTY:
        {
            memset(&IncomingData.api.networkDifficulty, 0, sizeof(double));
            memcpy(&IncomingData.api.networkDifficulty, &buffer[4], __min(dataLen, sizeof(double)));
            Serial.printf("Network Difficulty received: %f\n", IncomingData.api.networkDifficulty);
            break;
        }
        case LVGL_REG_API_BLOCK_HEIGHT:
        {
            memset(&IncomingData.api.blockHeight, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.blockHeight, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Block Height received: %lu\n", IncomingData.api.blockHeight);
            break;
        }
        case LVGL_REG_API_DIFFICULTY_PROGRESS:
        {
            memset(&IncomingData.api.difficultyProgressPercent, 0, sizeof(double));
            memcpy(&IncomingData.api.difficultyProgressPercent, &buffer[4], __min(dataLen, sizeof(double)));
            Serial.printf("Difficulty Progress Percent received: %f\n", IncomingData.api.difficultyProgressPercent);
            break;
        }
        case LVGL_REG_API_DIFFICULTY_CHANGE:
        {
            memset(&IncomingData.api.difficultyChangePercent, 0, sizeof(double));
            memcpy(&IncomingData.api.difficultyChangePercent, &buffer[4], __min(dataLen, sizeof(double)));
            Serial.printf("Difficulty Change Percent received: %f\n", IncomingData.api.difficultyChangePercent);
            break;
        }
        case LVGL_REG_API_REMAINING_BLOCKS:
        {
            memset(&IncomingData.api.remainingBlocksToDifficultyAdjustment, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.remainingBlocksToDifficultyAdjustment, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Remaining Blocks to Difficulty Adjustment received: %lu\n", IncomingData.api.remainingBlocksToDifficultyAdjustment);
            break;
        }
        case LVGL_REG_API_REMAINING_TIME:
        {
            memset(&IncomingData.api.remainingTimeToDifficultyAdjustment, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.remainingTimeToDifficultyAdjustment, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            IncomingData.api.remainingTimeToDifficultyAdjustment /= 1000; // Convert from milliseconds to seconds?
            Serial.printf("Remaining Time to Difficulty Adjustment received: %lu\n", IncomingData.api.remainingTimeToDifficultyAdjustment);
            break;
        }
        case LVGL_REG_API_FASTEST_FEE:
        {
            memset(&IncomingData.api.fastestFee, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.fastestFee, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Fastest Fee received: %lu\n", IncomingData.api.fastestFee);
            break;
        }
        case LVGL_REG_API_HALF_HOUR_FEE:
        {
            memset(&IncomingData.api.halfHourFee, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.halfHourFee, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Half Hour Fee received: %lu\n", IncomingData.api.halfHourFee);
            break;
        }
        case LVGL_REG_API_HOUR_FEE:
        {
            memset(&IncomingData.api.hourFee, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.hourFee, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Hour Fee received: %lu\n", IncomingData.api.hourFee);
            break;
        }
        case LVGL_REG_API_ECONOMY_FEE:
        {
            memset(&IncomingData.api.economyFee, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.economyFee, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Economy Fee received: %lu\n", IncomingData.api.economyFee);
            break;
        }
        case LVGL_REG_API_MINIMUM_FEE:
        {
            memset(&IncomingData.api.minimumFee, 0, MAX_UINT32_SIZE);
            memcpy(&IncomingData.api.minimumFee, &buffer[4], __min(dataLen, MAX_UINT32_SIZE));
            Serial.printf("Minimum Fee received: %lu\n", IncomingData.api.minimumFee);
            break;
        }
        default:
        {
            Serial.printf("Warning: Unknown API register 0x%02X with length %d\n", reg, dataLen);
            break;
        }
    }
}

void handleFlagsDataSerial(uint8_t* buffer, uint8_t len)
{
    //if (len < 2) return;
    
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];

    switch(reg)
    {
        case LVGL_FLAG_STARTUP_DONE:
        {
            specialRegisters.startupDone = buffer[4];
            Serial.printf("Startup Done flag received: %d\n", specialRegisters.startupDone);
            break;
        }
        case LVGL_FLAG_OVERHEAT_MODE:
        {
            specialRegisters.overheatMode = buffer[4];  // Keep raw incoming data
            bool currentFlag = buffer[4];
            
            if (currentFlag) {
                overheatModeCounter++;
                if (overheatModeCounter >= FLAG_CONFIRMATION_COUNT && !confirmedOverheatMode) {
                    confirmedOverheatMode = true;
                    ESP_LOGI("BAP", "Overheat Mode confirmed after %d consecutive readings", FLAG_CONFIRMATION_COUNT);
                }
            } else {
                overheatModeCounter = 0;
                if (confirmedOverheatMode) {
                    confirmedOverheatMode = false;
                    ESP_LOGI("BAP", "Overheat Mode cleared");
                }
            }
            ESP_LOGD("BAP", "Overheat Mode reading: %d, counter: %d, confirmed: %d", 
                     currentFlag, overheatModeCounter, confirmedOverheatMode);
            break;
        }
        case LVGL_FLAG_FOUND_BLOCK:
        {
            specialRegisters.foundBlock = buffer[4];  // Keep raw incoming data
            bool currentFlag = buffer[4];
            
            if (currentFlag) {
                foundBlockCounter++;
                if (foundBlockCounter >= FLAG_CONFIRMATION_COUNT && !confirmedFoundBlock) {
                    confirmedFoundBlock = true;
                    ESP_LOGI("BAP", "Found Block confirmed after %d consecutive readings", FLAG_CONFIRMATION_COUNT);
                }
            } else {
                foundBlockCounter = 0;
                if (confirmedFoundBlock) {
                    confirmedFoundBlock = false;
                    ESP_LOGI("BAP", "Found Block cleared");
                }
            }
            ESP_LOGD("BAP", "Found Block reading: %d, counter: %d, confirmed: %d", 
                     currentFlag, foundBlockCounter, confirmedFoundBlock);
            break;
        }
        default:
        {
            Serial.printf("Warning: Unknown flags register 0x%02X with length %d\n", reg, dataLen);
            break;
        }
    }
}   

void handleSpecialRegistersSerial(uint8_t* buffer, uint8_t len)
{
    uint8_t reg = buffer[2];
    uint8_t dataLen = buffer[3];

    switch(reg)
    {
        case LVGL_REG_SPECIAL_THEME:
        {
            memset(IncomingData.status.theme, 0, MAX_THEME_LENGTH);
            memcpy(IncomingData.status.theme, &buffer[4], __min(dataLen, MAX_THEME_LENGTH - 1));
            IncomingData.status.theme[dataLen] = '\0';  // Null terminate the string
            ESP_LOGI("BAP", "Theme received: %s", IncomingData.status.theme);
            break;
        }
        case LVGL_REG_SPECIAL_PRESET:
        {
            memset(IncomingData.status. preset, 0, MAX_PRESET_LENGTH);
            memcpy(IncomingData.status.preset, &buffer[4], __min(dataLen, MAX_PRESET_LENGTH - 1));
            IncomingData.status.preset[dataLen] = '\0';  // Null terminate the string
            ESP_LOGI("BAP", "Preset received: %s", IncomingData.status.preset);
            // save to NVS
            saveSettingsToNVSasString(NVS_KEY_ASIC_PRESET_NAME, IncomingData.status.preset, sizeof(IncomingData.status.preset));
            // update UI

            // uncheck all modes
            lv_obj_clear_state(quietMode, LV_STATE_CHECKED);
            lv_obj_clear_state(balancedMode, LV_STATE_CHECKED);
            lv_obj_clear_state(turboMode, LV_STATE_CHECKED);
            if (strcmp(IncomingData.status.preset, "quiet") == 0) {
                lv_obj_add_state(quietMode, LV_STATE_CHECKED);
            } else if (strcmp(IncomingData.status.preset, "balanced") == 0) {
                lv_obj_add_state(balancedMode, LV_STATE_CHECKED);
            } else if (strcmp(IncomingData.status.preset, "turbo") == 0) {
                lv_obj_add_state(turboMode, LV_STATE_CHECKED);
            }
            break;

        }
        default:
        {
            ESP_LOGW("BAP", "Unknown special register 0x%02X with length %d", reg, dataLen);
            break;
        }
    }
}

void sendRestartToBAP()
{
    if (specialRegisters.restart == 1)
    {
        writeDataToBAP(&specialRegisters.restart, sizeof(specialRegisters.restart), LVGL_REG_SPECIAL_RESTART);
        specialRegisters.restart = 0;
        Serial.println("Restart command sent to BAP");
        delay(5000);
        esp_restart();
    }
    else
    {
        Serial.println("No restart command to send to BAP");
    }
    return;
}


