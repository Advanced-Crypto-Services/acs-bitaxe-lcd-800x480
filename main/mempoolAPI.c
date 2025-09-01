#include "mempoolAPI.h"
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_http_client.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"


#define MAX_RESPONSE_SIZE 1024  // 8KB max response size

static char *responseBuffer = NULL;
static size_t responseLength = 0;

// Declare without initialization
MempoolApiState MEMPOOL_STATE;

void initializeResponseBuffer(void)
{
    responseBuffer = (char *)malloc(MAX_RESPONSE_SIZE);
    if (responseBuffer == NULL) {
        ESP_LOGE("MempoolAPI", "Failed to allocate memory for response buffer");
    }
}

// Add initialization function
void initializeMempoolState(void) {
    // Initialize logging
    esp_log_level_set("HTTP API", ESP_LOG_INFO);
    esp_log_level_set("MempoolAPI", ESP_LOG_INFO);
    
    memset(&MEMPOOL_STATE, 0, sizeof(MempoolApiState));
    // All numeric values are already 0 from memset
    // Just need to set boolean values to false
    MEMPOOL_STATE.priceValid = false;
    MEMPOOL_STATE.blockHeightValid = false;
    MEMPOOL_STATE.networkHashrateValid = false;
    MEMPOOL_STATE.networkDifficultyValid = false;
    MEMPOOL_STATE.difficultyProgressPercentValid = false;
    MEMPOOL_STATE.difficultyChangePercentValid = false;
    MEMPOOL_STATE.remainingBlocksToDifficultyAdjustmentValid = false;
    MEMPOOL_STATE.remainingTimeToDifficultyAdjustmentValid = false;
    MEMPOOL_STATE.fastestFeeValid = false;
    MEMPOOL_STATE.halfHourFeeValid = false;
    MEMPOOL_STATE.hourFeeValid = false;
    MEMPOOL_STATE.economyFeeValid = false;
    MEMPOOL_STATE.minimumFeeValid = false;
    MEMPOOL_STATE.addressValid = false;
}

// Add getter function implementation
MempoolApiState* getMempoolState(void) {
    return &MEMPOOL_STATE;
}

static esp_err_t priceEventHandler(esp_http_client_event_t *evt) 

/* Expected JSON format
{
  time: 1703252411,
  USD: 43753,
  EUR: 40545,
  GBP: 37528,
  CAD: 58123,
  CHF: 37438,
  AUD: 64499,
  JPY: 6218915
}
*/
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE("HTTP API", "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            if (responseBuffer != NULL) 
            {
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
        case HTTP_EVENT_HEADERS_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_REDIRECT:
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI("HTTP API", "Receiving price data chunk: %d bytes", evt->data_len);

            if (responseBuffer == NULL) 
            {
                responseBuffer = malloc(MAX_RESPONSE_SIZE);
                if (responseBuffer == NULL) 
                {
                    ESP_LOGE("HTTP API", "Initial memory allocation failed!");
                    responseLength = 0;
                    return ESP_ERR_NO_MEM;
                }
                // Clear the buffer
                memset(responseBuffer, 0, MAX_RESPONSE_SIZE);
                responseLength = 0;  // Reset length when allocating new buffer
            }
            
            if (responseLength + evt->data_len > MAX_RESPONSE_SIZE) {
                ESP_LOGE("HTTP API", "Response too large, maximum size is %d bytes", MAX_RESPONSE_SIZE);
                if (responseBuffer != NULL) {
                    free(responseBuffer);
                    responseBuffer = NULL;
                }
                responseLength = 0;
                return ESP_ERR_NO_MEM;
            }

            if (responseBuffer == NULL) {
                responseBuffer = malloc(MAX_RESPONSE_SIZE);
                if (responseBuffer == NULL) {
                    ESP_LOGE("HTTP API", "Initial memory allocation failed!");
                    responseLength = 0;
                    return ESP_ERR_NO_MEM;
                }
            }

            memcpy(responseBuffer + responseLength, evt->data, evt->data_len);
            responseLength += evt->data_len;
            responseBuffer[responseLength] = '\0';
            break;

        case HTTP_EVENT_ON_FINISH:
            if (responseBuffer != NULL) {
                cJSON *json = cJSON_Parse(responseBuffer);
                if (json != NULL) {
                    cJSON *priceUSD = cJSON_GetObjectItem(json, "USD");
                    if (priceUSD != NULL && cJSON_IsNumber(priceUSD)) {
                        MEMPOOL_STATE.priceUSD = priceUSD->valueint;
                        MEMPOOL_STATE.priceValid = true;
                        ESP_LOGE("HTTP API", "Price USD: %d", priceUSD->valueint);
                    }
                    cJSON *priceEUR = cJSON_GetObjectItem(json, "EUR");
                    if (priceEUR != NULL && cJSON_IsNumber(priceEUR)) {
                        MEMPOOL_STATE.priceEUR = priceEUR->valueint;
                        MEMPOOL_STATE.priceEURValid = true;
                        ESP_LOGE("HTTP API", "Price EUR: %d", priceEUR->valueint);
                    }
                    cJSON *priceCAD = cJSON_GetObjectItem(json, "CAD");
                    if (priceCAD != NULL && cJSON_IsNumber(priceCAD)) {
                        MEMPOOL_STATE.priceCAD = priceCAD->valueint;
                        MEMPOOL_STATE.priceCADValid = true;
                        ESP_LOGE("HTTP API", "Price CAD: %d", priceCAD->valueint);
                    }
                    cJSON_Delete(json);
                }
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
            
        case HTTP_EVENT_DISCONNECTED:
            if (responseBuffer != NULL) {
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
    }
    return ESP_OK;
}

static esp_err_t addressValidEventHandler(esp_http_client_event_t *evt) 

/* Expected JSON format
{
  time: 1703252411,
  USD: 43753,
  EUR: 40545,
  GBP: 37528,
  CAD: 58123,
  CHF: 37438,
  AUD: 64499,
  JPY: 6218915
}
*/
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE("HTTP API", "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            if (responseBuffer != NULL) 
            {
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
        case HTTP_EVENT_HEADERS_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_REDIRECT:
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI("HTTP API", "Receiving price data chunk: %d bytes", evt->data_len);

            if (responseBuffer == NULL) 
            {
                responseBuffer = malloc(MAX_RESPONSE_SIZE);
                if (responseBuffer == NULL) 
                {
                    ESP_LOGE("HTTP API", "Initial memory allocation failed!");
                    responseLength = 0;
                    return ESP_ERR_NO_MEM;
                }
                // Clear the buffer
                memset(responseBuffer, 0, MAX_RESPONSE_SIZE);
                responseLength = 0;  // Reset length when allocating new buffer
            }
            
            if (responseLength + evt->data_len > MAX_RESPONSE_SIZE) {
                ESP_LOGE("HTTP API", "Response too large, maximum size is %d bytes", MAX_RESPONSE_SIZE);
                if (responseBuffer != NULL) {
                    free(responseBuffer);
                    responseBuffer = NULL;
                }
                responseLength = 0;
                return ESP_ERR_NO_MEM;
            }

            if (responseBuffer == NULL) {
                responseBuffer = malloc(MAX_RESPONSE_SIZE);
                if (responseBuffer == NULL) {
                    ESP_LOGE("HTTP API", "Initial memory allocation failed!");
                    responseLength = 0;
                    return ESP_ERR_NO_MEM;
                }
            }

            memcpy(responseBuffer + responseLength, evt->data, evt->data_len);
            responseLength += evt->data_len;
            responseBuffer[responseLength] = '\0';
            break;

        case HTTP_EVENT_ON_FINISH:
            if (responseBuffer != NULL) {
                cJSON *json = cJSON_Parse(responseBuffer);
                if (json != NULL) {
                    cJSON *addressValid = cJSON_GetObjectItem(json, "isvalid");
                    if (addressValid != NULL && cJSON_IsBool(addressValid)) {
                        MEMPOOL_STATE.addressValid = (addressValid->type == cJSON_True);
                        ESP_LOGE("HTTP API", "Address Valid: %d", MEMPOOL_STATE.addressValid);
                    }
                    cJSON_Delete(json);
                }
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
            
        case HTTP_EVENT_DISCONNECTED:
            if (responseBuffer != NULL) {
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
    }
    return ESP_OK;
}

static esp_err_t networkSummaryEventHandler(esp_http_client_event_t *evt) 

/* Expected JSON format
{
"difficultyAdjustment":
    {"progressPercent":7.738095238095238,
    "difficultyChange":4.800834381577257,
    "estimatedRetargetDate":1736666192720,
    "remainingBlocks":1860,
    "remainingTime":1066992720,
    "previousRetarget":1.160520865570021,
    "nextRetargetHeight":878976,
    "timeAvg":576044,
    "cache_ttl":86400},
"hashrate":
    {"current":894536453617417600000,
    "currentDifficulty":109782075598905.2,
    "unit":"H/s","cache_ttl":86400},
"blockHeight":
    {"height":877126,"cache_ttl":300},
"fees":
    {"fastestFee":2,
    "halfHourFee":2,
    "hourFee":2,
    "economyFee":2,
    "minimumFee":1,
    "unit":"sat/vB",
    "cache_ttl":900},
"prices":
    {"time":1735601225,
    "USD":92447,
    "EUR":88819,
    "GBP":73689,
    "CAD":133090,
    "CHF":83503,
    "AUD":148924,
    "JPY":14546629,
    "cache_ttl":60},
"cached_at":"2024-12-30T23:27:43.691Z"}
*/
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE("HTTP API", "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            if (responseBuffer != NULL) 
            {
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
        case HTTP_EVENT_HEADERS_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_REDIRECT:
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI("HTTP API", "Receiving price data chunk: %d bytes", evt->data_len);

            if (responseBuffer == NULL) 
            {
                responseBuffer = malloc(MAX_RESPONSE_SIZE);
                if (responseBuffer == NULL) 
                {
                    ESP_LOGE("HTTP API", "Initial memory allocation failed!");
                    responseLength = 0;
                    return ESP_ERR_NO_MEM;
                }
                // Clear the buffer
                memset(responseBuffer, 0, MAX_RESPONSE_SIZE);
                responseLength = 0;  // Reset length when allocating new buffer
            }
            
            if (responseLength + evt->data_len > MAX_RESPONSE_SIZE) {
                ESP_LOGE("HTTP API", "Response too large, maximum size is %d bytes", MAX_RESPONSE_SIZE);
                if (responseBuffer != NULL) {
                    free(responseBuffer);
                    responseBuffer = NULL;
                }
                responseLength = 0;
                return ESP_ERR_NO_MEM;
            }

            if (responseBuffer == NULL) {
                responseBuffer = malloc(MAX_RESPONSE_SIZE);
                if (responseBuffer == NULL) {
                    ESP_LOGE("HTTP API", "Initial memory allocation failed!");
                    responseLength = 0;
                    return ESP_ERR_NO_MEM;
                }
            }

            memcpy(responseBuffer + responseLength, evt->data, evt->data_len);
            responseLength += evt->data_len;
            responseBuffer[responseLength] = '\0';
            break;

        case HTTP_EVENT_ON_FINISH:
            if (responseBuffer != NULL) {
                cJSON *json = cJSON_Parse(responseBuffer);
                if (json != NULL) {
                    // Parse difficulty adjustment
                    cJSON *diffAdj = cJSON_GetObjectItem(json, "difficultyAdjustment");
                    if (diffAdj != NULL) {
                        cJSON *progressPercent = cJSON_GetObjectItem(diffAdj, "progressPercent");
                        cJSON *diffChange = cJSON_GetObjectItem(diffAdj, "difficultyChange");
                        cJSON *remainingBlocks = cJSON_GetObjectItem(diffAdj, "remainingBlocks");
                        cJSON *remainingTime = cJSON_GetObjectItem(diffAdj, "remainingTime");

                        if (progressPercent && cJSON_IsNumber(progressPercent)) {
                            MEMPOOL_STATE.difficultyProgressPercent = progressPercent->valuedouble;
                            MEMPOOL_STATE.difficultyProgressPercentValid = true;
                            ESP_LOGI("HTTP API", "Progress Percent: %.2f%%", MEMPOOL_STATE.difficultyProgressPercent);
                        }
                        if (diffChange && cJSON_IsNumber(diffChange)) {
                            MEMPOOL_STATE.difficultyChangePercent = diffChange->valuedouble;
                            MEMPOOL_STATE.difficultyChangePercentValid = true;
                            ESP_LOGI("HTTP API", "Difficulty Change: %.2f%%", MEMPOOL_STATE.difficultyChangePercent);
                        }
                        if (remainingBlocks && cJSON_IsNumber(remainingBlocks)) {
                            MEMPOOL_STATE.remainingBlocksToDifficultyAdjustment = remainingBlocks->valueint;
                            MEMPOOL_STATE.remainingBlocksToDifficultyAdjustmentValid = true;
                            ESP_LOGI("HTTP API", "Remaining Blocks: %lu", MEMPOOL_STATE.remainingBlocksToDifficultyAdjustment);
                        }
                        if (remainingTime && cJSON_IsNumber(remainingTime)) {
                            MEMPOOL_STATE.remainingTimeToDifficultyAdjustment = remainingTime->valueint / 1000;
                            MEMPOOL_STATE.remainingTimeToDifficultyAdjustmentValid = true;
                            ESP_LOGI("HTTP API", "Remaining Time: %lu", MEMPOOL_STATE.remainingTimeToDifficultyAdjustment);
                        }
                    }

                    // Parse hashrate
                    cJSON *hashrate = cJSON_GetObjectItem(json, "hashrate");
                    if (hashrate != NULL) {
                        cJSON *current = cJSON_GetObjectItem(hashrate, "current");
                        cJSON *currentDifficulty = cJSON_GetObjectItem(hashrate, "currentDifficulty");

                        if (current && cJSON_IsNumber(current)) {
                            MEMPOOL_STATE.networkHashrate = current->valuedouble;
                            MEMPOOL_STATE.networkHashrateValid = true;
                            ESP_LOGI("HTTP API", "Network Hashrate: %.2f H/s", MEMPOOL_STATE.networkHashrate);
                        }
                        if (currentDifficulty && cJSON_IsNumber(currentDifficulty)) {
                            MEMPOOL_STATE.networkDifficulty = currentDifficulty->valuedouble;
                            MEMPOOL_STATE.networkDifficultyValid = true;
                            ESP_LOGI("HTTP API", "Network Difficulty: %.2f", MEMPOOL_STATE.networkDifficulty);
                        }
                    }

                    // Parse block height
                    cJSON *blockHeight = cJSON_GetObjectItem(json, "blockHeight");
                    if (blockHeight != NULL) {
                        cJSON *height = cJSON_GetObjectItem(blockHeight, "height");
                        if (height && cJSON_IsNumber(height)) {
                            MEMPOOL_STATE.blockHeight = height->valueint;
                            MEMPOOL_STATE.blockHeightValid = true;
                            ESP_LOGI("HTTP API", "Block Height: %lu", MEMPOOL_STATE.blockHeight);
                        }
                    }

                    // Parse fees
                    cJSON *fees = cJSON_GetObjectItem(json, "fees");
                    if (fees != NULL) {
                        cJSON *fastestFee = cJSON_GetObjectItem(fees, "fastestFee");
                        cJSON *halfHourFee = cJSON_GetObjectItem(fees, "halfHourFee");
                        cJSON *hourFee = cJSON_GetObjectItem(fees, "hourFee");
                        cJSON *economyFee = cJSON_GetObjectItem(fees, "economyFee");
                        cJSON *minimumFee = cJSON_GetObjectItem(fees, "minimumFee");

                        if (fastestFee && cJSON_IsNumber(fastestFee)) {
                            MEMPOOL_STATE.fastestFee = fastestFee->valueint;
                            MEMPOOL_STATE.fastestFeeValid = true;
                            ESP_LOGI("HTTP API", "Fastest Fee: %lu sat/vB", MEMPOOL_STATE.fastestFee);
                        }
                        if (halfHourFee && cJSON_IsNumber(halfHourFee)) {
                            MEMPOOL_STATE.halfHourFee = halfHourFee->valueint;
                            MEMPOOL_STATE.halfHourFeeValid = true;
                            ESP_LOGI("HTTP API", "Half Hour Fee: %lu sat/vB", MEMPOOL_STATE.halfHourFee);
                        }
                        if (hourFee && cJSON_IsNumber(hourFee)) {
                            MEMPOOL_STATE.hourFee = hourFee->valueint;
                            MEMPOOL_STATE.hourFeeValid = true;
                            ESP_LOGI("HTTP API", "Hour Fee: %lu sat/vB", MEMPOOL_STATE.hourFee);
                        }
                        if (economyFee && cJSON_IsNumber(economyFee)) {
                            MEMPOOL_STATE.economyFee = economyFee->valueint;
                            MEMPOOL_STATE.economyFeeValid = true;
                            ESP_LOGI("HTTP API", "Economy Fee: %lu sat/vB", MEMPOOL_STATE.economyFee);
                        }
                        if (minimumFee && cJSON_IsNumber(minimumFee)) {
                            MEMPOOL_STATE.minimumFee = minimumFee->valueint;
                            MEMPOOL_STATE.minimumFeeValid = true;
                            ESP_LOGI("HTTP API", "Minimum Fee: %lu sat/vB", MEMPOOL_STATE.minimumFee);
                        }
                    }

                    // Parse prices (if needed, though you might want to use the dedicated price endpoint)
                    cJSON *prices = cJSON_GetObjectItem(json, "prices");
                    cJSON *priceCAD = cJSON_GetObjectItem(prices, "CAD");
                    cJSON *priceEUR = cJSON_GetObjectItem(prices, "EUR");
                    if (prices != NULL) {
                        cJSON *priceUSD = cJSON_GetObjectItem(prices, "USD");
                        if (priceUSD && cJSON_IsNumber(priceUSD)) {
                            MEMPOOL_STATE.priceUSD = priceUSD->valueint;
                            MEMPOOL_STATE.priceValid = true;
                            ESP_LOGI("HTTP API", "Price USD: %lu", MEMPOOL_STATE.priceUSD);
                        }
                        if (priceCAD && cJSON_IsNumber(priceCAD)) {
                            MEMPOOL_STATE.priceCAD = priceCAD->valueint;
                            MEMPOOL_STATE.priceCADValid = true;
                            ESP_LOGI("HTTP API", "Price CAD: %lu", MEMPOOL_STATE.priceCAD);
                        }
                        if (priceEUR && cJSON_IsNumber(priceEUR)) {
                            MEMPOOL_STATE.priceEUR = priceEUR->valueint;
                            MEMPOOL_STATE.priceEURValid = true;
                            ESP_LOGI("HTTP API", "Price EUR: %lu", MEMPOOL_STATE.priceEUR);
                        }
                        
                    }
                    

                    cJSON_Delete(json);
                }
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
            
        case HTTP_EVENT_DISCONNECTED:
            if (responseBuffer != NULL) {
                free(responseBuffer);
                responseBuffer = NULL;
                responseLength = 0;
            }
            break;
    }
    return ESP_OK;
}


// Update the mempool_api_price function
esp_err_t mempool_api_price(void) {
    static int64_t lastUpdate = 0;  
    int64_t currentTime = esp_timer_get_time();
    
    // Check if it's time to send the request. Limit the number of requests here
    if (((currentTime - lastUpdate)/1000000) > 5 || lastUpdate == 0)
    {
        lastUpdate = currentTime;
        // Set up the HTTP client configuration using esp cert bndles
        //Using Coingecko API key for testing
        // TODO: move this to own server for production to limit the number of requests
        esp_http_client_config_t config = {
            .url = "https://mempool.space/api/v1/prices",
            .event_handler = priceEventHandler,  // Use specific handler
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .crt_bundle_attach = esp_crt_bundle_attach,
        };

        // Initialize client
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL) 
        {
            ESP_LOGE("HTTP API", "Failed to initialize HTTP client");
            return ESP_FAIL;
        }

        /* // Set headers with error checking Not needed for this API
        esp_err_t err;
        if ((err = esp_http_client_set_header(client, "accept", "application/json")) != ESP_OK) 
        {
            ESP_LOGE("HTTP API", "Failed to set accept header: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            return err;
        }

        if ((err = esp_http_client_set_header(client, "x-cg-demo-api-key", "CG-iUyVPGbu2nECCwVo8yXXUf57")) != ESP_OK) 
        {
            ESP_LOGE("HTTP API", "Failed to set API key header: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            return err;
        }*/

        // Perform request 
        esp_err_t err;
        err = esp_http_client_perform(client);
        
        if (err == ESP_OK) 
        {
            ESP_LOGE("HTTP API", "HTTP GET Status = %d", esp_http_client_get_status_code(client));
        } 
        else 
        {
            ESP_LOGE("HTTP API", "HTTP GET request failed: %s", esp_err_to_name(err));
        }
        
        esp_http_client_cleanup(client);
        return err;
    }
    
    return ESP_OK;
}

esp_err_t mempool_api_address_valid(const char* bitcoinAddress) {
    static int64_t lastUpdate = 0;  
    int64_t currentTime = esp_timer_get_time();
    
    if (((currentTime - lastUpdate)/1000000) > 2|| lastUpdate == 0)
    {
        lastUpdate = currentTime;

        // Create the full URL with the bitcoin address
        char fullUrl[256];
        snprintf(fullUrl, sizeof(fullUrl), "https://mempool.space/api/v1/validate-address/%s", bitcoinAddress);
        
        esp_http_client_config_t config = {
            .url = fullUrl,  // Use the constructed URL
            .event_handler = addressValidEventHandler,
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .crt_bundle_attach = esp_crt_bundle_attach,
            .buffer_size = 1024,  // Reduce buffer size
            .buffer_size_tx = 1024,
            .disable_auto_redirect = true,  // Disable redirects to save memory
        };

        // Initialize client
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL) 
        {
            ESP_LOGE("HTTP API", "Failed to initialize HTTP client");
            return ESP_FAIL;
        }

        /* // Set headers with error checking Not needed for this API
        esp_err_t err;
        if ((err = esp_http_client_set_header(client, "accept", "application/json")) != ESP_OK) 
        {
            ESP_LOGE("HTTP API", "Failed to set accept header: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            return err;
        }

        if ((err = esp_http_client_set_header(client, "x-cg-demo-api-key", "CG-iUyVPGbu2nECCwVo8yXXUf57")) != ESP_OK) 
        {
            ESP_LOGE("HTTP API", "Failed to set API key header: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            return err;
        }*/

        // Perform request 
        esp_err_t err;
        err = esp_http_client_perform(client);
        
        if (err == ESP_OK) 
        {
            ESP_LOGE("HTTP API", "HTTP GET Status = %d", esp_http_client_get_status_code(client));
        } 
        else 
        {
            ESP_LOGE("HTTP API", "HTTP GET request failed: %s", esp_err_to_name(err));
        }
        
        esp_http_client_cleanup(client);
        return err;
    }
    
    return ESP_OK;
}

esp_err_t mempool_api_network_summary(void) 
{
    static int64_t lastUpdate = 0;  
    int64_t currentTime = esp_timer_get_time();
    
    // Check if it's time to send the request. Limit the number of requests here
    if (((currentTime - lastUpdate)/1000000) > 90 || lastUpdate == 0) // 90 seconds between requests
    {
        lastUpdate = currentTime;
        // Set up the HTTP client configuration using esp cert bndles
        //Using Coingecko API key for testing
        // TODO: move this to own server for production to limit the number of requests
        esp_http_client_config_t config = {
            .url = "https://d2pvkeh6egkhn1.cloudfront.net/api/v1/network-summary",
            .event_handler = networkSummaryEventHandler,  // Use specific handler
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .crt_bundle_attach = esp_crt_bundle_attach,
            .buffer_size = 1024,  // Reduce buffer size
            .buffer_size_tx = 1024,
            .disable_auto_redirect = true,  // Disable redirects to save memory
        };

        // Initialize client
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL) 
        {
            ESP_LOGE("HTTP API", "Failed to initialize HTTP client");
            return ESP_FAIL;
        }

        /* // Set headers with error checking Not needed for this API
        esp_err_t err;
        if ((err = esp_http_client_set_header(client, "accept", "application/json")) != ESP_OK) 
        {
            ESP_LOGE("HTTP API", "Failed to set accept header: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            return err;
        }

        if ((err = esp_http_client_set_header(client, "x-cg-demo-api-key", "CG-iUyVPGbu2nECCwVo8yXXUf57")) != ESP_OK) 
        {
            ESP_LOGE("HTTP API", "Failed to set API key header: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            return err;
        }*/

        // Perform request 
        esp_err_t err;
        err = esp_http_client_perform(client);
        
        if (err == ESP_OK) 
        {
            ESP_LOGE("HTTP API", "HTTP GET Status = %d", esp_http_client_get_status_code(client));
        } 
        else 
        {
            ESP_LOGE("HTTP API", "HTTP GET request failed: %s", esp_err_to_name(err));
        }
        
        esp_http_client_cleanup(client);
        return err;
    }
    
    return ESP_OK;
}