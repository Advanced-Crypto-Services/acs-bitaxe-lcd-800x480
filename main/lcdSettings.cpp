#include "lcdSettings.h"
#include <Arduino.h>

#define ACS_BLPWM 6
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

bool backlightPWM = false;


void initializeIOExpander(void)
{
//Initialize IO Expander
    pinMode(GPIO_INPUT_IO_4, OUTPUT); /// ctp irq
    
    /**
     * These development boards require the use of an IO expander to configure the screen,
     * so it needs to be initialized in advance and registered with the panel for use.
     *
     */
    ESP_LOGI("IO_EXPANDER", "Initialize IO expander");
    /* Initialize IO expander */
    
    ESP_IOExpander *expander = nullptr;
    esp_err_t err;

    // First try Waveshare IO expander (CH422G)
    ESP_LOGI("IO_EXPANDER", "Trying Waveshare CH422G expander...");
    expander = new ESP_IOExpander_CH422G((i2c_port_t)I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
    expander->init();
    if (expander->begin() != ESP_OK) {
        ESP_LOGI("IO_EXPANDER", "CH422G expander failed - trying ACS expander");
        delete expander;
        
        // Try ACS IO Expander (TCA95xx)
        expander = new ESP_IOExpander_TCA95xx_8bit((i2c_port_t)I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
        expander->init();
        if (expander->begin() != ESP_OK) {
            ESP_LOGE("IO_EXPANDER", "ACS TCA95xx expander failed");
            delete expander;
            expander = nullptr;
        } else {
            ESP_LOGI("IO_EXPANDER", "ACS TCA95xx expander initialized successfully");
            pinMode(ACS_BLPWM, OUTPUT); /// ACS Bitaxe Touch LCD can adjust backlight brightneses
            backlightPWM = true;
        }
    } else {
        ESP_LOGI("IO_EXPANDER", "CH422G expander initialized successfully");
        backlightPWM = false;
    }

    if (expander != nullptr) {
        expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
        expander->multiDigitalWrite(TP_RST | LCD_BL | LCD_RST, HIGH);
        
        //gt911 initialization
        expander->multiDigitalWrite(TP_RST | LCD_RST, LOW);
        digitalWrite(GPIO_INPUT_IO_4, LOW);
        expander->multiDigitalWrite(TP_RST | LCD_RST, HIGH);
    } else {
        ESP_LOGE("IO_EXPANDER", "Failed to initialize any IO expander");
        // Handle the case where no IO expander is available
    }
if (backlightPWM) {
    // Configure LEDC timer
    ledc_timer_config_t bkPWMConfig = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 100000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&bkPWMConfig));

    // Configure LEDC channel
    ledc_channel_config_t bkPWMChannelConfig = {
        .gpio_num = ACS_BLPWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 255,  // Start at full brightness
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&bkPWMChannelConfig));
    ESP_LOGI("Backlight", "PWM initialized");
}
}
