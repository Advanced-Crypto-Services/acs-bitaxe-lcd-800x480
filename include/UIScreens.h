#pragma once

#include <lvgl.h>
#include "wifiFeatures.h"

#define SMOOTHING_WINDOW_SIZE 5



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
    lv_obj_t* bitcoinNewsMainContainer;
    lv_obj_t* settingsMainContainer;
    lv_timer_t* labelUpdateTimer;
    lv_timer_t* chartUpdateTimer;
    lv_timer_t* statusBarUpdateTimer;
    lv_timer_t* clockTimer;
    lv_timer_t* apiUpdateTimer;
    lv_timer_t* autoTuneSettingsTimer;

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
extern lv_timer_t* chartUpdateTimer;
extern lv_timer_t* networkUpdateTimer;
extern lv_timer_t* autoTuneSettingsTimer;

extern lv_obj_t* splashScreenContainer;

extern float calculateMovingAverage(float newValue);
extern void hashrateGraph(lv_obj_t* parent);



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
