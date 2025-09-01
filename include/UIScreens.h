#pragma once

#include <lvgl.h>
#include "wifiFeatures.h"

#define SMOOTHING_WINDOW_SIZE 5
#define GRAPH_MAX_VARIATIONS 3  // Maximum number of graph types per container

extern const lv_img_dsc_t Logos;
extern const lv_img_dsc_t Splash;
extern const lv_img_dsc_t UIBackground;
extern const lv_img_dsc_t openSourceBitcoinMining;
extern const lv_img_dsc_t bitcoin;
extern const lv_img_dsc_t activity;
extern const lv_img_dsc_t house;
extern const lv_img_dsc_t pickaxe;
extern const lv_img_dsc_t statusbar;
extern const lv_img_dsc_t bitcoin20by20;
extern const lv_img_dsc_t plugZap20by20;
extern const lv_img_dsc_t cpu20by20;
extern const lv_img_dsc_t wifi20by20;

extern lv_obj_t* tabHome;
extern lv_obj_t* tabMining;
extern lv_obj_t* tabActivity;
extern lv_obj_t* tabBitcoinNews;
extern lv_obj_t* tabSettings;
extern lv_obj_t* networkSettingsContainer;
extern lv_obj_t* networkSettingsContainerConnectedState;
extern lv_obj_t* webUIqrCode;
extern lv_obj_t* currentIPContainer;
extern lv_obj_t* webUIVersionContainer;
extern lv_obj_t* ssidContainer;
extern lv_obj_t* connectQrCodeContainer;
extern lv_obj_t* LCDotaAddressLabel;
extern lv_obj_t* LCDotaQRCode;
extern char LCDotaAddressLabelText[100];

extern lv_obj_t* quietMode;
extern lv_obj_t* balancedMode;
extern lv_obj_t* turboMode;

enum ScreenType 
{
    activeScreenSplash,
    activeScreenHome,
    activeScreenMining,
    activeScreenActivity,
    activeScreenBitcoinNews,
    activeScreenSettings,
    activeScreenInitStartup
};

struct ScreenObjects 
{
    lv_obj_t* background;
    lv_obj_t* statusBar;
    lv_obj_t* tabIcons;
    lv_obj_t* homeMainContainer;
    lv_obj_t* miningMainContainer;
    lv_obj_t* activityMainContainer;
    lv_obj_t* btcStatsMainContainer;
    lv_obj_t* settingsMainContainer;
    lv_timer_t* deviceStatsLabelsUpdateTimer;
    lv_timer_t* miningStatsLabelsUpdateTimer;
    lv_timer_t* statusBarUpdateTimer;
    lv_timer_t* clockTimer;
    lv_timer_t* autoTuneSettingsTimer;
    lv_timer_t* btcStatsLabelsUpdateTimer;
    lv_timer_t* chartUpdateTimer;
    lv_obj_t* settingTabView;
};

// Declare the struct type This is currently unused
struct SettingsTextAreas {
    lv_obj_t* hostnameTextArea;
    lv_obj_t* wifiTextArea;
    lv_obj_t* wifiPasswordTextArea;
    lv_obj_t* stratumUrlTextArea;
    lv_obj_t* stratumPortTextArea;
    lv_obj_t* stratumUserTextArea;
    lv_obj_t* stratumPasswordTextArea;
    lv_obj_t* stratumUrlTextAreaFallback;
    lv_obj_t* stratumPortTextAreaFallback;
    lv_obj_t* stratumUserTextAreaFallback;
    lv_obj_t* stratumPasswordTextAreaFallback;
    lv_obj_t* asicFrequencyTextArea;
    lv_obj_t* asicVoltageTextArea;
};

extern SettingsTextAreas settingsTextAreas;
extern ScreenObjects screenObjs;
extern WifiNetworkScan* storedNetworks;
extern uint16_t* storedNetworkCount;

// global variables
extern ScreenType activeScreen;
extern lv_timer_t* networkUpdateTimer;
extern lv_timer_t* autoTuneSettingsTimer;

extern lv_obj_t* splashScreenContainer;

extern float calculateMovingAverage(float newValue);

// graph buffer
extern float hashrateBuffer[SMOOTHING_WINDOW_SIZE];
extern int bufferIndex;

extern void splashScreen();
extern void homeScreen();
extern void miningStatusScreen();
extern void initalizeOneScreen();
extern void bitcoinNewsScreen();
extern void activityScreen();
extern void settingsScreen();
extern void showSettingsConfirmationOverlay();
extern void showOverheatOverlay();
extern void showBlockFoundOverlay();
extern void setupSettingsScreen();

extern void quadContainer(lv_obj_t* parentMainContainer, lv_obj_t*& container1, lv_obj_t*& container2, lv_obj_t*& container3, lv_obj_t*& container4);
extern void quadContainerContent(lv_obj_t* container1, const char* Title, const char* Value);

extern void graphContainer(lv_obj_t* parentMainContainer, lv_obj_t*& container);
extern void graphContainerContent(lv_obj_t* container, const char* title, const char* currentValue, const char* bottomLeftLabel, const char* bottomRightLabel);
