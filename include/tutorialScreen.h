#pragma once

#include <lvgl.h>
#include <esp_wifi.h>
#include "esp_event.h"
#include "wifiFeatures.h"

enum tutorialScreens {
    tutorialScreenHome,
    tutorialScreenLeftTab,
    tutorialScreenStatusBar,
    tutorialScreenWebUI
};

extern void tutorialScreen();
extern void switchToTutorialScreen(tutorialScreens newScreen);

