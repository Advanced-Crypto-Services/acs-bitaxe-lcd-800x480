#include "UIScreens.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include <Arduino.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "UIHandler.h"
#include "UISharedAssets.h"
#include "fonts.h"
#include <TimeLib.h>
#include "BAP.h"
#include "UIThemes.h"
#include "NVS.h"
#include "initialSetupScreen.h"
#include "UISharedAssets.h"
#include "UIScreens.h"
#include <esp_wifi.h>
#include <WiFi.h>
#include "mempoolAPI.h"
#include "fonts.h"
#include "BAP.h"
#include "NVS.h"
#include "modelPresets.h"


#define __min(a, b) ((a) < (b) ? (a) : (b));

extern WebServer* setupServer;
extern String setupBTCAddress;

constexpr size_t MAX_WIFI_SSID_LENGTH = 64;      // WPA2 max SSID length
constexpr size_t MAX_WIFI_PASSWORD_LENGTH = 64;   // WPA2 max password length

char tempWifiSSID[MAX_WIFI_SSID_LENGTH + 1];     // +1 for null terminator
char tempWifiPassword[MAX_WIFI_PASSWORD_LENGTH + 1];


initialSetupScreens activeSetupScreen = initialSetupScreenSplash;

// Container references for each setup screen
static lv_obj_t* setupBackground = NULL;
static lv_obj_t* setupSplashContainer = NULL;
static lv_obj_t* setupWifiContainer = NULL;
static lv_obj_t* setupThemeContainer = NULL;
static lv_obj_t* setupPoolContainer = NULL;
static lv_obj_t* setupPoolUserContainer = NULL;
static lv_obj_t* setupCompleteContainer = NULL;
static lv_obj_t* setupScreenContainer = NULL;
static lv_obj_t* btcLabel = NULL;
static lv_obj_t* wifiDiscoveredNetworksContainer = NULL;
static lv_obj_t* wifiList = NULL;
static lv_obj_t* wifiSsidTextArea = NULL;
static lv_obj_t* wifiPasswordTextArea = NULL;
static lv_obj_t* backtoWifiSelectionButton = NULL;
static lv_obj_t* confirmWifiBtn = NULL;
static lv_obj_t* btcAddressTextArea = NULL; 
static lv_obj_t* pool1Container = NULL;
static lv_obj_t* connectingLabel = NULL;
static bool initialSetupComplete = false;
static lv_obj_t* setupKeyboard = NULL;
static bool cleanupComplete = false;  // Add this with other static variables at top
static bool wifiConnected = false;
static lv_timer_t* httpServerTimer = nullptr;
static lv_obj_t* httpServerQRCodeContainer = NULL;
static lv_obj_t* setupLaterButton = NULL;
static lv_obj_t* setupLaterButtonLabel = NULL;
static lv_obj_t* backButton = NULL;
static lv_obj_t* backButtonLabel = NULL;
static lv_obj_t* wifiRescanButton = NULL;

static void httpServerQRCode(lv_obj_t* parent);
void initSetupSplashScreen();
void initSetupWifiScreen();
static void initSetupQRCodeScreen(lv_obj_t* parent);
void scanAndDisplayNetworks(lv_obj_t* parent);
static void ta_event_cb(lv_event_t * e);
static void createKeyboard();
static void backButtonEventHandler(lv_event_t* event);

static void finishSetup(const char* btcAddress);

static void finishSetup(const char* btcAddress)
{
    Serial0.println("Address is valid");
         //check if address is not null
        if (btcAddress != NULL)
        {
            lv_label_set_text(btcLabel, "Address is valid");
            saveSettingsToNVSasString(NVS_KEY_BTC_ADDRESS, btcAddress, strlen(btcAddress));

            delay(20);
            specialRegisters.restart = 0;
            // memset URL, USER, Password, port
            memset(BAPStratumUrlMainBuffer, 0, BAP_STRATUM_URL_MAIN_BUFFER_SIZE);
            memset(BAPStratumUserMainBuffer, 0, BAP_STRATUM_USER_MAIN_BUFFER_SIZE);
            memset(BAPStratumPassMainBuffer, 0, BAP_STRATUM_PASS_MAIN_BUFFER_SIZE);
            memset(BAPStratumPortMainBuffer, 0, BAP_STRATUM_PORT_MAIN_BUFFER_SIZE);
            delay(20);
            memset(BAPStratumUrlFallbackBuffer, 0, BAP_STRATUM_URL_FALLBACK_BUFFER_SIZE);
            memset(BAPStratumUserFallbackBuffer, 0, BAP_STRATUM_USER_FALLBACK_BUFFER_SIZE);
            memset(BAPStratumPassFallbackBuffer, 0, BAP_STRATUM_PASS_FALLBACK_BUFFER_SIZE);
            memset(BAPStratumPortFallbackBuffer, 0, BAP_STRATUM_PORT_FALLBACK_BUFFER_SIZE);
            delay(20);
            // set URL, USER, Password primary
            strcpy((char*)BAPStratumUrlMainBuffer, "public-pool.io");
            Serial0.printf("BAPStratumUrlMainBuffer: %s\n", (char*)BAPStratumUrlMainBuffer);
            snprintf((char*)BAPStratumUserMainBuffer, BAP_STRATUM_USER_MAIN_BUFFER_SIZE, "%s.ACSBitaxeTouch", btcAddress);
            strcpy((char*)BAPStratumPortMainBuffer, "21496");
            Serial0.printf("BAPStratumPortMainBuffer: %s\n", (char*)BAPStratumPortMainBuffer);
            Serial0.printf("BAPStratumUserMainBuffer: %s\n", (char*)BAPStratumUserMainBuffer);
            strcpy((char*)BAPStratumPassMainBuffer, "x");
            Serial0.printf("BAPStratumPassMainBuffer: %s\n", (char*)BAPStratumPassMainBuffer);

            // set URL, USER, Password fallback
            strcpy((char*)BAPStratumUrlFallbackBuffer, "solo.ckpool.org");
            Serial0.printf("BAPStratumUrlFallbackBuffer: %s\n", (char*)BAPStratumUrlFallbackBuffer);
            snprintf((char*)BAPStratumUserFallbackBuffer, BAP_STRATUM_USER_FALLBACK_BUFFER_SIZE, "%s.ACSBitaxeTouch", btcAddress);
            Serial0.printf("BAPStratumUserFallbackBuffer: %s\n", (char*)BAPStratumUserFallbackBuffer);
            strcpy((char*)BAPStratumPassFallbackBuffer, "x");
            Serial0.printf("BAPStratumPassFallbackBuffer: %s\n", (char*)BAPStratumPassFallbackBuffer);
            // Set port Primary (ensure consistent endianness)
            uint16_t port = 21496;  // Use uint16_t as ports are 16-bit values

            BAPStratumPortMainBuffer[0] = (port >> 8) & 0xFF;  // High byte
            BAPStratumPortMainBuffer[1] = port & 0xFF;         // Low byte
            writeDataToBAP(BAPStratumPortMainBuffer, sizeof(uint16_t), BAP_STRATUM_PORT_MAIN_BUFFER_REG);
            Serial0.printf("BAPStratumPortMainBuffer: %d\n", port);

            // Set port Fallback (ensure consistent endianness)
            uint16_t portFallback = 3333;  // Use uint16_t as ports are 16-bit values
            BAPStratumPortFallbackBuffer[0] = (portFallback >> 8) & 0xFF;  // High byte
            BAPStratumPortFallbackBuffer[1] = portFallback & 0xFF;         // Low byte
            writeDataToBAP(BAPStratumPortFallbackBuffer, sizeof(uint16_t), BAP_STRATUM_PORT_FALLBACK_BUFFER_REG);
            Serial0.printf("BAPStratumPortFallbackBuffer: %d\n", portFallback);

            // send Pool Settings to BAP
            // Public-Pool.io URL
            writeDataToBAP((uint8_t*)BAPStratumUrlMainBuffer, strlen((char*)BAPStratumUrlMainBuffer), BAP_STRATUM_URL_MAIN_BUFFER_REG);
            // Public-Pool.io USER
            writeDataToBAP((uint8_t*)BAPStratumUserMainBuffer, strlen((char*)BAPStratumUserMainBuffer), BAP_STRATUM_USER_MAIN_BUFFER_REG);
            // Public-Pool.io PASSWORD
            writeDataToBAP((uint8_t*)BAPStratumPassMainBuffer, strlen((char*)BAPStratumPassMainBuffer), BAP_STRATUM_PASS_MAIN_BUFFER_REG);
            // Public-Pool.io PORT
            writeDataToBAP((uint8_t*)BAPStratumPortMainBuffer, sizeof(BAPStratumPortMainBuffer), BAP_STRATUM_PORT_MAIN_BUFFER_REG);
            // CK pool fallback URL
            writeDataToBAP((uint8_t*)BAPStratumUrlFallbackBuffer, strlen((char*)BAPStratumUrlFallbackBuffer), BAP_STRATUM_URL_FALLBACK_BUFFER_REG);
            // CK pool fallback USER
            writeDataToBAP((uint8_t*)BAPStratumUserFallbackBuffer, strlen((char*)BAPStratumUserFallbackBuffer), BAP_STRATUM_USER_FALLBACK_BUFFER_REG);
            // CK pool fallback PASSWORD
            writeDataToBAP((uint8_t*)BAPStratumPassFallbackBuffer, strlen((char*)BAPStratumPassFallbackBuffer), BAP_STRATUM_PASS_FALLBACK_BUFFER_REG);
            // CK pool fallback PORT
            writeDataToBAP((uint8_t*)BAPStratumPortFallbackBuffer, sizeof(BAPStratumPortFallbackBuffer), BAP_STRATUM_PORT_FALLBACK_BUFFER_REG);
        }
        // set asic settings
            setBalancedPreset(); 
            delay(200);
            // Create new splash screen while keeping old screens hidden
            Serial0.println("Creating new splash screen");
            splashScreen();
            // refresh splash screen
            lv_refr_now(NULL);
            Serial0.println("New splash screen created");
            delay(20);
            // set first boot to true
            setFirstBootComplete();
            delay(200);
             // Restart Bitaxe 
            Serial0.printf("Restarting Bitaxe\n");
            specialRegisters.restart = 1;
            writeDataToBAP(&specialRegisters.restart, sizeof(specialRegisters.restart), LVGL_REG_SPECIAL_RESTART);
            delay(200);
            specialRegisters.restart = 0;
            // delay for stability
            delay(2000);
            Serial0.println("Delaying for stability");
            
            // Now that new screen is stable, trigger cleanup
            Serial0.println("Triggering cleanup");
            esp_restart();
            initialSetupComplete = true;
            // reset tutorial completed
            saveSettingsToNVSasU16(NVS_KEY_TUTORIAL_COMPLETED, 0);
        }


 static lv_obj_t* initSetupTextAreaStyles(lv_obj_t* parent, const char* placeholder)
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
    return ta;
}

 static void initSetupCursorStyles(lv_obj_t * ta)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_set_style_bg_color(ta, theme->textColor, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(ta, LV_OPA_100, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(ta, 0, LV_PART_CURSOR | LV_STATE_FOCUSED);  // Remove border completely
}


static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(setupKeyboard != NULL) {
            lv_keyboard_set_textarea(setupKeyboard, ta);
            lv_obj_clear_flag(setupKeyboard, LV_OBJ_FLAG_HIDDEN);
            // Disable animation for showing keyboard
            lv_obj_set_style_anim_time(setupKeyboard, 0, 0);
            if (ta == settingsTextAreas.stratumPortTextArea || ta == settingsTextAreas.stratumPortTextAreaFallback ||
                ta == settingsTextAreas.asicVoltageTextArea || ta == settingsTextAreas.asicFrequencyTextArea) 
            {
                lv_keyboard_set_mode(setupKeyboard, LV_KEYBOARD_MODE_NUMBER);
                lv_obj_set_style_text_font(setupKeyboard, LV_FONT_DEFAULT, LV_PART_ITEMS); 
            }
            else
            {
                lv_keyboard_set_mode(setupKeyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
        }
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED) {
        // Disable animation for hiding keyboard
        lv_obj_set_style_anim_time(setupKeyboard, 0, 0);
        lv_obj_add_flag(setupKeyboard, LV_OBJ_FLAG_HIDDEN);
    }
}


void switchToInitialSetupScreen(initialSetupScreens newScreen) {
    if (activeSetupScreen == newScreen) {
        return; // Already on this screen
    }

    lvgl_port_lock(-1);  // Lock for thread safety

    // Hide all containers first
    if (setupSplashContainer) lv_obj_add_flag(setupSplashContainer, LV_OBJ_FLAG_HIDDEN);
    if (setupWifiContainer) lv_obj_add_flag(setupWifiContainer, LV_OBJ_FLAG_HIDDEN);
    if (setupThemeContainer) lv_obj_add_flag(setupThemeContainer, LV_OBJ_FLAG_HIDDEN);
    if (setupPoolContainer) lv_obj_add_flag(setupPoolContainer, LV_OBJ_FLAG_HIDDEN);
    if (setupPoolUserContainer) lv_obj_add_flag(setupPoolUserContainer, LV_OBJ_FLAG_HIDDEN);
    if (setupCompleteContainer) lv_obj_add_flag(setupCompleteContainer, LV_OBJ_FLAG_HIDDEN);

    // Show the new container and create it if it doesn't exist
    switch (newScreen) {
        case initialSetupScreenSplash:
            if (!setupSplashContainer) {
                // Create splash screen container and content
                initSetupSplashScreen();
            }
            lv_obj_clear_flag(setupSplashContainer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(backButton, LV_OBJ_FLAG_HIDDEN);
            break;
            
        case initialSetupScreenSetupWifi:
            if (!setupWifiContainer) {
                // Create WiFi setup container and content
                setupWifiContainer = lv_obj_create(setupScreenContainer);
                
                // Add WiFi setup specific content
            }
            lv_obj_clear_flag(setupWifiContainer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(backButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(backButton);
            break;

        case initialSetupScreenSetupPool:
            if (!setupPoolContainer) {
                // Create Pool setup container and content
                initSetupPoolScreen();
            }
            lv_obj_clear_flag(setupPoolContainer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(backButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(backButton);
            break;

            
        // Add similar cases for other setup screens
    }

    activeSetupScreen = newScreen;
    lvgl_port_unlock();
}

void initialSetupScreen()
{
    uiTheme_t* theme = getCurrentTheme();
    lvgl_port_lock(-1);

    // Create and load the background screen
    setupBackground = lv_obj_create(NULL);
    lv_obj_set_size(setupBackground, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(setupBackground, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(setupBackground, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(setupBackground, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(setupBackground, theme->backgroundColor, LV_PART_MAIN);
    lv_scr_load(setupBackground);

    // Create main container for setup screens
    setupScreenContainer = lv_obj_create(setupBackground);
    lv_obj_set_size(setupScreenContainer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(setupScreenContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(setupScreenContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_opa(setupScreenContainer, 0, LV_PART_MAIN);
    
    // create back button
    backButton = lv_btn_create(setupScreenContainer);
    lv_obj_set_size(backButton, 128, 48);
    lv_obj_align(backButton, LV_ALIGN_TOP_LEFT, 8, 16);
    lv_obj_add_flag(backButton, LV_OBJ_FLAG_CLICKABLE);
    backButtonLabel = lv_label_create(backButton);
    lv_label_set_text(backButtonLabel, "Back");
    lv_obj_set_style_text_font(backButtonLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(backButtonLabel, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(backButtonLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(backButton, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(backButton, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(backButtonLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(backButton, backButtonEventHandler, LV_EVENT_CLICKED, NULL);
    //hide back button
    lv_obj_add_flag(backButton, LV_OBJ_FLAG_HIDDEN);

    // Initialize all setup screens but keep them hidden
    initSetupSplashScreen();
    initSetupWifiScreen();
    initSetupPoolScreen();
    lv_obj_add_flag(setupSplashContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(setupWifiContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(setupPoolContainer, LV_OBJ_FLAG_HIDDEN);
    // clear scrollable flag
    lv_obj_clear_flag(setupScreenContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(setupSplashContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(setupWifiContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(setupPoolContainer, LV_OBJ_FLAG_SCROLLABLE);

    // Create keyboard
    createKeyboard();
    lv_obj_add_flag(setupKeyboard, LV_OBJ_FLAG_HIDDEN);
    
    // Show initial splash screen
    lv_obj_clear_flag(setupSplashContainer, LV_OBJ_FLAG_HIDDEN);
    
    lvgl_port_unlock();
        // delete splash screen container if it exists after new screen is loaded
    if (splashScreenContainer != NULL)
    {
        if (splashScreenContainer != NULL)
        {
            lv_obj_del(splashScreenContainer);
        }
    }
    
    // Block until setup is complete, but handle LVGL updates
    while (!initialSetupComplete) 
    {
        delay(100);  // Small delay to prevent tight looping
    }

}

static void setupSplashScreenEventHandler(lv_event_t* event)
{
    switchToInitialSetupScreen(initialSetupScreenSetupWifi);
    Serial0.println("switching to wifi setup screen");
}

// Init Setup Splash Screen
void initSetupSplashScreen()
{
    uiTheme_t* theme = getCurrentTheme();
    // create splash screen container
    setupSplashContainer = lv_obj_create(setupScreenContainer);
    lv_obj_set_size(setupSplashContainer, 800, 480);
    lv_obj_align(setupSplashContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(setupSplashContainer, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(setupSplashContainer, LV_OPA_100, LV_PART_MAIN);
    
    lv_obj_set_style_border_width(setupSplashContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(setupSplashContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(setupSplashContainer, LV_OBJ_FLAG_SCROLLABLE);
    // create splash screen content
    // Top Label
    lv_obj_t* splashScreenLabel = lv_label_create(setupSplashContainer);
    lv_label_set_text(splashScreenLabel, "Welcome\nLet's Get Started");
    lv_obj_set_style_text_font(splashScreenLabel, theme->fontExtraBold56, LV_PART_MAIN);
    lv_obj_set_style_text_align(splashScreenLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(splashScreenLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(splashScreenLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(splashScreenLabel, LV_ALIGN_CENTER, 0, -88);

    /*
    // Logo 1   
    lv_obj_t* osbmLogo = lv_img_create(setupSplashContainer);
    lv_img_set_src(osbmLogo, theme->logo2);
    lv_obj_center(osbmLogo);

    // Logo 2
    lv_obj_t* logos = lv_img_create(setupSplashContainer);
    lv_img_set_src(logos, theme->logo1);
    lv_obj_align(logos, LV_ALIGN_BOTTOM_MID, 0, -100);
    */

    // button with Label
    lv_obj_t* splashScreenButtonContainer = lv_obj_create(setupSplashContainer);
    lv_obj_set_size(splashScreenButtonContainer, 256, 96);
    lv_obj_add_flag(splashScreenButtonContainer, LV_OBJ_FLAG_CLICKABLE);

    //label
    lv_obj_t* splashScreenButtonLabel = lv_label_create(splashScreenButtonContainer);
    lv_label_set_text(splashScreenButtonLabel, "Begin Setup");
    lv_obj_set_style_text_font(splashScreenButtonLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(splashScreenButtonLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(splashScreenButtonLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(splashScreenButtonLabel, LV_ALIGN_CENTER, 16, 0);

    // start symbol
    lv_obj_t* startSymbol = lv_label_create(splashScreenButtonContainer);
    lv_label_set_text(startSymbol, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(startSymbol, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(startSymbol, LV_OPA_0, LV_PART_MAIN);
    lv_obj_align(startSymbol, LV_ALIGN_LEFT_MID, 16, 0);

    // Set alignment and layout
    lv_obj_set_style_text_align(splashScreenButtonContainer, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    //lv_obj_set_style_align(splashScreenBottomLabel, LV_ALIGN_CENTER, LV_PART_MAIN);
    // Add padding to center verticall

    lv_obj_set_style_bg_color(splashScreenButtonContainer, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(splashScreenButtonContainer, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_radius(splashScreenButtonContainer, 16, LV_PART_MAIN);
    lv_obj_set_style_text_color(splashScreenButtonContainer, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(splashScreenButtonContainer, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(splashScreenButtonContainer, LV_ALIGN_CENTER, 0, 72);
    lv_obj_add_event_cb(splashScreenButtonContainer, setupSplashScreenEventHandler, LV_EVENT_CLICKED, NULL);

    // QR Code Container
    initSetupQRCodeScreen(setupSplashContainer);




    // show splash screen
}

// Callback when a network is selected from the list
static void wifiListEventHandler(lv_event_t* e) {
    lv_obj_t* list = lv_event_get_target(e);
    lv_obj_t* selectedBtn = lv_event_get_target(e);
    const char* selectedSsid = lv_list_get_btn_text(list, selectedBtn);
    strncpy(tempWifiSSID, selectedSsid, sizeof(tempWifiSSID) - 1);
    tempWifiSSID[sizeof(tempWifiSSID) - 1] = '\0'; // Ensure null termination
    lv_obj_add_flag(wifiList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifiRescanButton, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifiSsidTextArea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifiPasswordTextArea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(confirmWifiBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(backtoWifiSelectionButton, LV_OBJ_FLAG_HIDDEN);

    lv_textarea_set_text(wifiSsidTextArea, tempWifiSSID);

Serial0.println(tempWifiSSID);
}

static void confirmWifiBtnEventHandler(lv_event_t* e) {
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* btn = lv_event_get_target(e);
    Serial0.println("confirmWifiBtnEventHandler");

    // Create "Testing connection..." label
    lvgl_port_lock(-1);

    if (connectingLabel != NULL) {
        lv_obj_del(connectingLabel);
    }
    lv_refr_now(NULL);
    lvgl_port_unlock();

    lvgl_port_lock(-1);
    connectingLabel = lv_label_create(setupWifiContainer);
    lv_obj_set_style_text_font(connectingLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(connectingLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(connectingLabel, LV_ALIGN_CENTER, 0, 40);
    lv_label_set_text(connectingLabel, "Testing connection...");
    delay(100);
    lv_refr_now(NULL);
    lvgl_port_unlock();
    delay(100);

    // Get credentials from text areas
    strncpy(tempWifiSSID, lv_textarea_get_text(wifiSsidTextArea), sizeof(tempWifiSSID) - 1);
    tempWifiSSID[sizeof(tempWifiSSID) - 1] = '\0';

    strncpy(tempWifiPassword, lv_textarea_get_text(wifiPasswordTextArea), sizeof(tempWifiPassword) - 1);
    tempWifiPassword[sizeof(tempWifiPassword) - 1] = '\0';
    

    Serial0.printf("SSID: %s\n", tempWifiSSID);
    Serial0.printf("Password: %s\n", tempWifiPassword);

    WiFi.begin(tempWifiSSID, tempWifiPassword);
    Serial0.printf("Connecting to %s\n", tempWifiSSID);
    unsigned long currentTime = millis();
    while (millis() - currentTime < 10000) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial0.printf("Connected to %s\n", tempWifiSSID);
            wifiConnected = true;
            break;
        }
        delay(100);
        Serial0.printf(".");
    }
    
    if (!wifiConnected) {
        Serial0.printf("Failed to connect to %s\n", tempWifiSSID);
        wifiConnected = false;
    }

    if (wifiConnected) {
        lv_label_set_text(connectingLabel, "Connection successful");
        httpServerQRCode(setupPoolContainer);

        // Save to NVS
        saveSettingsToNVSasString(NVS_KEY_WIFI_SSID1, tempWifiSSID, strlen(tempWifiSSID));
        saveSettingsToNVSasString(NVS_KEY_WIFI_PASSWORD1, tempWifiPassword, strlen(tempWifiPassword));

        // Set first boot to false
        

        // Send to BAP Port
        writeDataToBAP((uint8_t*)tempWifiSSID, strlen(tempWifiSSID), BAP_WIFI_SSID_BUFFER_REG);
        delay(500);
        writeDataToBAP((uint8_t*)tempWifiPassword, strlen(tempWifiPassword), BAP_WIFI_PASS_BUFFER_REG);
        delay(20);
        switchToInitialSetupScreen(initialSetupScreenSetupPool);

    }
    else {
        lv_label_set_text(connectingLabel, "Connection failed\n Check Password");
    }


    
}

static void rescanButtonEventHandler(lv_event_t* e) {
    Serial0.println("rescanButtonEventHandler");
    uiTheme_t* theme = getCurrentTheme();
    // Clear existing items from the list
    lv_obj_clean(wifiList);
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
            lv_obj_t* btn = lv_list_add_btn(wifiList, 
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
            lv_obj_add_event_cb(btn, wifiListEventHandler, LV_EVENT_CLICKED, NULL);

            // Get the image from the button and recolor it
            lv_obj_t* img = lv_obj_get_child(btn, 0);  // First child is the image
            lv_obj_set_style_img_recolor(img, theme->primaryColor, LV_PART_MAIN);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        }
    } else {
        // Add a message if no networks are found
        lv_list_add_text(wifiList, "No networks found");
    }
    Serial0.println("wifiList Populated");

}

static void backtoWifiSelectionButtonEventHandler(lv_event_t* e) {
    Serial0.println("backtoWifiSelectionButtonEventHandler");
    lv_obj_clear_flag(wifiList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifiSsidTextArea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifiPasswordTextArea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(confirmWifiBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(backtoWifiSelectionButton, LV_OBJ_FLAG_HIDDEN);
    if (connectingLabel != NULL) {
        lv_obj_add_flag(connectingLabel, LV_OBJ_FLAG_HIDDEN);
    }
}
//initial setup wifi screen
void initSetupWifiScreen()
{
 uiTheme_t* theme = getCurrentTheme();
    // create splash screen container
    setupWifiContainer = lv_obj_create(setupScreenContainer);
    lv_obj_set_size(setupWifiContainer, 800, 480);
    lv_obj_align(setupWifiContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(setupWifiContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(setupWifiContainer, LV_OPA_0, LV_PART_MAIN);

    //wifi discovered networks container
    wifiDiscoveredNetworksContainer = lv_obj_create(setupWifiContainer);
    lv_obj_set_size(wifiDiscoveredNetworksContainer, 448, 400);
    lv_obj_align(wifiDiscoveredNetworksContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(wifiDiscoveredNetworksContainer, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wifiDiscoveredNetworksContainer, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(wifiDiscoveredNetworksContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(wifiDiscoveredNetworksContainer, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_opa(wifiDiscoveredNetworksContainer, LV_OPA_100, LV_PART_MAIN);
    // clear scrollable flag
    lv_obj_clear_flag(wifiDiscoveredNetworksContainer, LV_OBJ_FLAG_SCROLLABLE);
    Serial0.println("initSetupWifiScreen");

    //wifi discovered networks label
    lv_obj_t* wifiDiscoveredNetworksLabel = lv_label_create(wifiDiscoveredNetworksContainer);
    lv_label_set_text(wifiDiscoveredNetworksLabel, "Wifi");
    lv_obj_set_style_text_font(wifiDiscoveredNetworksLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifiDiscoveredNetworksLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(wifiDiscoveredNetworksLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(wifiDiscoveredNetworksLabel, LV_ALIGN_TOP_MID, 0, 24);
    Serial0.println("wifiDiscoveredNetworksLabel");

    // wifi icon
    lv_obj_t* wifiIcon = lv_img_create(wifiDiscoveredNetworksContainer);
     lv_img_set_src(wifiIcon, "S:/wifi40x40.png");
    lv_obj_set_style_bg_opa(wifiIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(wifiIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(wifiIcon, LV_OPA_COVER, LV_PART_MAIN);
    //lv_img_set_zoom(wifiIcon, 512);
    lv_obj_align(wifiIcon, LV_ALIGN_TOP_MID, 0, -24);
    Serial0.println("wifiIcon Created");

    //wifi discovered networks list

    wifiList = lv_list_create(wifiDiscoveredNetworksContainer);
    lv_obj_set_size(wifiList, 424, 240);
    lv_obj_align(wifiList, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(wifiList, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(wifiList, 0, LV_PART_MAIN);
    Serial0.println("wifiList Created");
    

    // Populate list with stored networks
    if (storedNetworks != nullptr && *storedNetworkCount > 0) {
        for (uint16_t i = 0; i < *storedNetworkCount; i++) {
            // Add list item with WiFi icon and SSID
            lv_obj_t* btn = lv_list_add_btn(wifiList, 
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
            lv_obj_add_event_cb(btn, wifiListEventHandler, LV_EVENT_CLICKED, NULL);

            // Get the image from the button and recolor it
            lv_obj_t* img = lv_obj_get_child(btn, 0);  // First child is the image
            lv_obj_set_style_img_recolor(img, theme->primaryColor, LV_PART_MAIN);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        }
    } else {
        // Add a message if no networks are found
        lv_list_add_text(wifiList, "No networks found");
    }
    Serial0.println("wifiList Populated");

    // Rescan Button
    wifiRescanButton = lv_btn_create(wifiDiscoveredNetworksContainer);
    lv_obj_set_size(wifiRescanButton, 128, 48);
    lv_obj_align(wifiRescanButton, LV_ALIGN_BOTTOM_MID, 0, 16);
    lv_obj_set_style_bg_color(wifiRescanButton, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wifiRescanButton, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_flag(wifiRescanButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(wifiRescanButton, rescanButtonEventHandler, LV_EVENT_CLICKED, NULL);

    //rescan button label
    lv_obj_t* wifiRescanButtonLabel = lv_label_create(wifiRescanButton);
    lv_label_set_text(wifiRescanButtonLabel, "Rescan");
    lv_obj_set_style_text_font(wifiRescanButtonLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifiRescanButtonLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(wifiRescanButtonLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_center(wifiRescanButtonLabel);
    

    
    // add hidden text fields for wifi ssid and password after chosen from the list
    wifiSsidTextArea = initSetupTextAreaStyles(wifiDiscoveredNetworksContainer, "SSID");
    initSetupCursorStyles(wifiSsidTextArea);
    lv_obj_set_size(wifiSsidTextArea, 260, 48);
    lv_obj_align(wifiSsidTextArea, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(wifiSsidTextArea, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wifiSsidTextArea, LV_OPA_80, LV_PART_MAIN);
    lv_obj_add_flag(wifiSsidTextArea, LV_OBJ_FLAG_HIDDEN);
    Serial0.println("wifiSsidTextArea Created");
    lv_obj_add_event_cb(wifiSsidTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    wifiPasswordTextArea = initSetupTextAreaStyles(wifiDiscoveredNetworksContainer, "Password");
    initSetupCursorStyles(wifiPasswordTextArea);
    lv_obj_set_size(wifiPasswordTextArea, 260, 48);
    lv_obj_align(wifiPasswordTextArea, LV_ALIGN_TOP_MID, 0, 144);
    lv_obj_set_style_bg_color(wifiPasswordTextArea, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wifiPasswordTextArea, LV_OPA_80, LV_PART_MAIN);
    lv_obj_add_flag(wifiPasswordTextArea, LV_OBJ_FLAG_HIDDEN);
    Serial0.println("wifiPasswordTextArea Created");

    // add a button to confirm the wifi setup
    confirmWifiBtn = lv_btn_create(wifiDiscoveredNetworksContainer);
    lv_obj_set_size(confirmWifiBtn, 128, 48);
    lv_obj_align(confirmWifiBtn, LV_ALIGN_BOTTOM_MID, 128, 0);
    lv_obj_set_style_bg_color(confirmWifiBtn, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(confirmWifiBtn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_flag(confirmWifiBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t* confirmWifiBtnLabel = lv_label_create(confirmWifiBtn);
    lv_obj_add_event_cb(confirmWifiBtn, confirmWifiBtnEventHandler, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(confirmWifiBtnLabel, "Confirm");
    lv_obj_set_style_text_font(confirmWifiBtnLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(confirmWifiBtnLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(confirmWifiBtnLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_center(confirmWifiBtnLabel);

    // add a Back button
    backtoWifiSelectionButton = lv_btn_create(wifiDiscoveredNetworksContainer);
    lv_obj_set_size(backtoWifiSelectionButton, 128, 48);
    lv_obj_align_to(backtoWifiSelectionButton, confirmWifiBtn, LV_ALIGN_LEFT_MID, -256, 0);
    lv_obj_set_style_bg_color(backtoWifiSelectionButton, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(backtoWifiSelectionButton, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_flag(backtoWifiSelectionButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(backtoWifiSelectionButton, backtoWifiSelectionButtonEventHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_t* backtoWifiSelectionButtonLabel = lv_label_create(backtoWifiSelectionButton);
    lv_label_set_text(backtoWifiSelectionButtonLabel, "Back");
    lv_obj_set_style_text_font(backtoWifiSelectionButtonLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(backtoWifiSelectionButtonLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(backtoWifiSelectionButtonLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_center(backtoWifiSelectionButtonLabel);
    lv_obj_add_flag(backtoWifiSelectionButton, LV_OBJ_FLAG_HIDDEN);

    // qr code container
    initSetupQRCodeScreen(setupWifiContainer);
    Serial0.println("qr code container Created");

    

}
static void confirmBtnEventHandler(lv_event_t* e) 
{
    Serial0.println("confirmBtnEventHandler");
    const char* btcAddress = lv_textarea_get_text(btcAddressTextArea);
    Serial0.printf("BTC Address: %s\n", btcAddress);
    // check if address is valid
    if (WiFi.status() == WL_CONNECTED) 
    {
        if (!MEMPOOL_STATE.addressValid)
        {
        lv_label_set_text(btcLabel, "Checking Address...");
        lv_refr_now(NULL);
        Serial0.println("WiFi is connected");
        mempool_api_address_valid(btcAddress);
        Serial0.printf("Address Valid: %d\n", MEMPOOL_STATE.addressValid);
        }
        if (MEMPOOL_STATE.addressValid) 
        {
            finishSetup(btcAddress);
        }
        else 
        {
        Serial0.println("Address is not valid");
        // show error message
        lv_label_set_text(btcLabel, "Address is not valid");

    
        }
    // save btc address to nvs
    // save pool to nvs
    // save wifi to nvs
    }
        
    else
    {
    Serial0.println("WiFi is disconnected");
    // show error message
    lv_label_set_text(btcLabel, "WiFi is disconnected");
    delay(1000);
    switchToInitialSetupScreen(initialSetupScreenSetupPool);
    }
}

static void showBtcAddressField(lv_obj_t* poolContainer) {
    uiTheme_t* theme = getCurrentTheme();
    
    lvgl_port_lock(-1);
    // Clear existing content except the pool label and image
    // We'll keep the first two children (assumed to be label and image)
    while(lv_obj_get_child_cnt(poolContainer) > 2) {
        lv_obj_t* child = lv_obj_get_child(poolContainer, 2);
        lv_obj_del(child);
    }



    // Create BTC address label
    btcLabel = lv_label_create(poolContainer);
    lv_label_set_text(btcLabel, "BTC Payout Address");
    lv_obj_set_style_text_font(btcLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcLabel, LV_ALIGN_TOP_MID, 0, 120);
    
    // Create text area for BTC address
     btcAddressTextArea = lv_textarea_create(poolContainer);
    lv_obj_set_size(btcAddressTextArea, 260, 72);
    lv_obj_align(btcAddressTextArea, LV_ALIGN_TOP_MID, 0,40 );
    lv_textarea_set_placeholder_text(btcAddressTextArea, "bc1...");
    //lv_textarea_set_text(btcAddressTextArea, "bc1qnp980s5fpp8l94p5cvttmtdqy8rvrq74qly2yrfmzkdsntqzlc5qkc4rkq");
    lv_obj_set_style_text_font(btcAddressTextArea, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcAddressTextArea, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btcAddressTextArea, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btcAddressTextArea, LV_OPA_80, LV_PART_MAIN);
    lv_obj_add_event_cb(btcAddressTextArea, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(btcAddressTextArea, theme->textColor, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(btcAddressTextArea, LV_OPA_100, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btcAddressTextArea, 0, LV_PART_CURSOR | LV_STATE_FOCUSED);  // Remove border completely


    
    // Create confirm button
    lv_obj_t* confirmBtn = lv_btn_create(poolContainer);
    lv_obj_set_size(confirmBtn, 160, 48);
    lv_obj_align(confirmBtn, LV_ALIGN_TOP_MID, 0, -16);
    lv_obj_set_style_bg_color(confirmBtn, theme->primaryColor, LV_PART_MAIN);
    lv_obj_add_flag(confirmBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(confirmBtn, confirmBtnEventHandler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* confirmLabel = lv_label_create(confirmBtn);
    lv_label_set_text(confirmLabel, "Confirm");
    lv_obj_set_style_text_font(confirmLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(confirmLabel, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(confirmLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_center(confirmLabel);
    lvgl_port_unlock();
    
}

static void pool1ContainerEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* poolContainer = lv_event_get_target(e);
    Serial0.println("pool1ContainerEventHandler");
    if(code == LV_EVENT_CLICKED) 
    {
        bool currentState = lv_obj_has_state(poolContainer, LV_STATE_CHECKED);
        Serial0.printf("Current state: %s\n", currentState ? "checked" : "unchecked");
        // Check if container is already checked to avoid re-creating fields
        if(lv_obj_has_state(poolContainer, LV_STATE_CHECKED)) {
            showBtcAddressField(poolContainer);
            Serial0.println("showing btc address field");
            return;
        }
        else {
            Serial0.println("clearing btc address field");
            while(lv_obj_get_child_cnt(poolContainer) > 2) {
                lv_obj_t* child = lv_obj_get_child(poolContainer, 2);
                lv_obj_del(child);
            }
            Serial0.println("cleared btc address fields");
        }
       
        
    }
}

static void setupLaterButtonEventHandler(lv_event_t* e)
{
    uiTheme_t* theme = getCurrentTheme();
    lvgl_port_lock(-1);
    lv_obj_set_style_bg_color(setupLaterButton, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_text_color(setupLaterButtonLabel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_refr_now(NULL);
    lvgl_port_unlock();
    delay(100);
    
    finishSetup(NULL);
}
static void httpServerTimerCallback(lv_timer_t* timer);
// Setup Pool Screen
void initSetupPoolScreen()
{
uiTheme_t* theme = getCurrentTheme();
    // Create HTTP server timer if it doesn't exist
    if (!httpServerTimer) {
        httpServerTimer = lv_timer_create(httpServerTimerCallback, 10, nullptr);  // 10ms interval
    }
    
    // create splash screen container
    setupPoolContainer = lv_obj_create(setupScreenContainer);
    lv_obj_set_size(setupPoolContainer, 800, 480);
    lv_obj_align(setupPoolContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(setupPoolContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(setupPoolContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(setupPoolContainer, LV_OBJ_FLAG_SCROLLABLE);

    // pool 1 container
     pool1Container = lv_obj_create(setupPoolContainer);
    lv_obj_set_size(pool1Container, 300, 400);
    lv_obj_align(pool1Container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(pool1Container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pool1Container, LV_OPA_100, LV_PART_MAIN);
    //lv_obj_add_flag(pool1Container, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_clear_flag(pool1Container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(pool1Container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(pool1Container, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_opa(pool1Container, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_radius(pool1Container, 16, LV_PART_MAIN);
    //lv_obj_add_event_cb(pool1Container, pool1ContainerEventHandler, LV_EVENT_CLICKED, NULL);
    showBtcAddressField(pool1Container);

    // style the checkable
    lv_obj_set_style_border_width(pool1Container, 8, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(pool1Container, lv_color_hex(0x21CCAB), LV_PART_MAIN | LV_STATE_CHECKED);

    // pool 1 container label
    lv_obj_t* pool1ContainerLabel = lv_label_create(pool1Container);
    lv_label_set_text(pool1ContainerLabel, "public-pool.io");
    lv_obj_set_style_text_font(pool1ContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(pool1ContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(pool1ContainerLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(pool1ContainerLabel, LV_ALIGN_BOTTOM_MID, 0, 16);


    // pool 1 container image
    lv_obj_t* pool1ContainerImage = lv_img_create(pool1Container);
    lv_img_set_src(pool1ContainerImage, "S:/publicPool200x200.png");
    lv_obj_align(pool1ContainerImage, LV_ALIGN_BOTTOM_MID, 0, 0);

    // setup later button
    setupLaterButton = lv_btn_create(setupPoolContainer);
    lv_obj_set_size(setupLaterButton, 128, 96);
    lv_obj_align(setupLaterButton, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(setupLaterButton, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(setupLaterButton, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_radius(setupLaterButton, 16, LV_PART_MAIN);
    lv_obj_add_flag(setupLaterButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(setupLaterButton, setupLaterButtonEventHandler, LV_EVENT_CLICKED, NULL);

    setupLaterButtonLabel = lv_label_create(setupLaterButton);
    lv_label_set_text(setupLaterButtonLabel, "Setup\n Later");
    lv_obj_set_style_text_font(setupLaterButtonLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(setupLaterButtonLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(setupLaterButtonLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_text_align(setupLaterButtonLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(setupLaterButtonLabel, LV_ALIGN_CENTER, 0, 0);

    //lv_obj_add_event_cb(setupLaterButton, setupLaterButtonEventHandler, LV_EVENT_CLICKED, NULL);

    // qr code container
    //initSetupQRCodeScreen(setupPoolContainer); // comment out because it was too crowded with the ip QR code
}


//QR Code Setup Screen
static void initSetupQRCodeScreen(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    // QR Code Container
    lv_obj_t* qrCodeContainer = lv_obj_create(parent);
    lv_obj_set_size(qrCodeContainer, 256, 224);
    //lv_obj_set_width(qrCodeContainer, lv_pct(45));
    lv_obj_align(qrCodeContainer, LV_ALIGN_BOTTOM_LEFT, -56, 8);
    lv_obj_set_style_bg_opa(qrCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(qrCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_color(qrCodeContainer, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(qrCodeContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCodeContainer, 16, LV_PART_MAIN);
    lv_obj_clear_flag(qrCodeContainer, LV_OBJ_FLAG_SCROLLABLE);

    // QR Code Label
    lv_obj_t* qrCodeLabel = lv_label_create(qrCodeContainer);
    lv_label_set_text(qrCodeLabel, "Scan QR\nfor help");
    lv_obj_set_style_text_font(qrCodeLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(qrCodeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(qrCodeLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(qrCodeLabel, LV_ALIGN_TOP_MID, 0, 0);

    // Setup QR Code
    const char* qrCodeText = "https://www.advancedcryptoservices.com/bitaxe-support";
    lv_obj_t* qrCode = lv_qrcode_create(qrCodeContainer,128,lv_color_hex(0xffffff),theme->backgroundColor);
    lv_qrcode_update(qrCode, qrCodeText, strlen(qrCodeText));
    lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 32);
}

static void httpServerQRCode(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    // QR Code Container
    httpServerQRCodeContainer = lv_obj_create(parent);
    String ipAddress = WiFi.localIP().toString();
    
    // Create a static buffer to store the URL
    static char qrCodeBuffer[64]; // Adjust size as needed
    snprintf(qrCodeBuffer, sizeof(qrCodeBuffer), "http://%s/", ipAddress.c_str());
    
    lv_obj_set_size(httpServerQRCodeContainer, 256, 264);
    lv_obj_align(httpServerQRCodeContainer, LV_ALIGN_BOTTOM_LEFT, -56, 16);
    lv_obj_set_style_bg_opa(httpServerQRCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(httpServerQRCodeContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_color(httpServerQRCodeContainer, theme->borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(httpServerQRCodeContainer, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(httpServerQRCodeContainer, 16, LV_PART_MAIN);
    lv_obj_clear_flag(httpServerQRCodeContainer, LV_OBJ_FLAG_SCROLLABLE);

    // QR Code Label
    lv_obj_t* httpServerQRCodeLabel = lv_label_create(httpServerQRCodeContainer);
    lv_label_set_text(httpServerQRCodeLabel, "Setup on\nYour Phone");
    lv_obj_set_style_text_font(httpServerQRCodeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(httpServerQRCodeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(httpServerQRCodeLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(httpServerQRCodeLabel, LV_ALIGN_TOP_MID, 0, 0);

    // IP Address Label
    lv_obj_t* ipAddressLabel = lv_label_create(httpServerQRCodeContainer);
    lv_label_set_text(ipAddressLabel, ipAddress.c_str());
    lv_obj_set_style_text_font(ipAddressLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ipAddressLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(ipAddressLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(ipAddressLabel, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Setup QR Code 
    lv_obj_t* qrCode = lv_qrcode_create(httpServerQRCodeContainer,128,lv_color_hex(0xffffff),theme->backgroundColor);
    lv_qrcode_update(qrCode, qrCodeBuffer, strlen(qrCodeBuffer));
    lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 16);
}


static void createKeyboard()
{
    // Create keyboard
    uiTheme_t* theme = getCurrentTheme();
    setupKeyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(setupKeyboard, LV_HOR_RES, LV_VER_RES / 2);
    // Remove all default styles
    lv_obj_remove_style_all(setupKeyboard);
    // Set keyboard background
    lv_obj_set_style_bg_color(setupKeyboard, theme->backgroundColor, LV_PART_MAIN);  // Dark background
    lv_obj_set_style_bg_opa(setupKeyboard, LV_OPA_80, LV_PART_MAIN);
    // Style for buttons (single state only)
    lv_obj_set_style_bg_color(setupKeyboard, theme->backgroundColor, LV_PART_ITEMS);  // Dark button background
    lv_obj_set_style_text_color(setupKeyboard, theme->textColor, LV_PART_ITEMS);  // Your theme color for text
    lv_obj_set_style_border_width(setupKeyboard, 2, LV_PART_ITEMS);
    lv_obj_set_style_border_color(setupKeyboard, theme->borderColor, LV_PART_ITEMS);
    lv_obj_set_style_radius(setupKeyboard, 24, LV_PART_ITEMS);
    // Regular keys
    lv_obj_set_style_text_font(setupKeyboard, theme->fontMedium16, LV_PART_ITEMS);
    // Symbols that Inter Font doesn't support
    lv_obj_set_style_text_font(setupKeyboard, LV_FONT_DEFAULT, LV_PART_ITEMS | LV_STATE_CHECKED);
    // Disable ALL animations and transitions
    lv_obj_set_style_anim_time(setupKeyboard, 0, 0);
    lv_obj_clear_flag(setupKeyboard, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_clear_flag(setupKeyboard, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(setupKeyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(setupKeyboard, LV_OBJ_FLAG_HIDDEN);
}

static void httpServerTimerCallback(lv_timer_t* timer) {
    if (setupServer) {
        setupServer->handleClient();
        if (btcAddressTextArea && !setupBTCAddress.isEmpty()) {
            lv_textarea_set_text(btcAddressTextArea, setupBTCAddress.c_str());
            setupBTCAddress.clear();
            delay(20);
            confirmBtnEventHandler(NULL);
        }
    }
}


static void backButtonEventHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI("Back Button", "Back button clicked");
        switch (activeSetupScreen)
        {
        case initialSetupScreenSplash:
            break;
        case initialSetupScreenSetupWifi:
            switchToInitialSetupScreen(initialSetupScreenSplash);
            break;
        case initialSetupScreenSetupPool:
            switchToInitialSetupScreen(initialSetupScreenSetupWifi);
            break;
        

        default:
            break;
        }
    }
}