#include "tutorialScreen.h"
#include "UIHandler.h"
#include "UIScreens.h"
#include <esp_log.h>
#include "UIThemes.h"
#include "homeScreen.h"
#include "UISharedAssets.h"
#include "NVS.h"
static const char* TAG = "TutorialScreen";

// Global variables
static lv_obj_t* tutorialOverlay = nullptr;
static lv_obj_t* tutorialTextHome = nullptr;
static lv_obj_t* tutorialTextLeftTab = nullptr;
static lv_obj_t* tutorialTextStatusBar = nullptr;
static lv_obj_t* tutorialTextWebUI = nullptr;
static lv_obj_t* tutorialHighlightHome = nullptr;
static lv_obj_t* tutorialHighlightLeftTab = nullptr;
static lv_obj_t* tutorialHighlightStatusBar = nullptr;
static lv_obj_t* tutorialHighlightWebUI = nullptr;
static lv_obj_t* tempWifiIcon = nullptr;

static tutorialScreens currentTutorialScreen = tutorialScreenHome;

// Move the function declaration to the top of the file, after the includes
static void tutorialStatusBarClickHandler(lv_event_t* e);
static void endTutorial();

// Function to update tutorial content based on current screen
static void updateTutorialContent() {
    uiTheme_t* theme = getCurrentTheme();
    
    switch (currentTutorialScreen) {
        case tutorialScreenHome:
            // hide status bar and left tab
            lv_obj_add_flag(statusBarObj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(leftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(tutorialTextHome, LV_OBJ_FLAG_HIDDEN);
            switchToScreen(activeScreenHome);
            lv_label_set_text(tutorialTextHome, "This is the Main Home screen Container. \nTouch the Container to cycle through screens.");
            
            // Highlight the home main container
            lv_obj_clear_flag(tutorialHighlightHome, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(tutorialHighlightHome, 
                lv_obj_get_x(screenObjs.homeMainContainer),
                lv_obj_get_y(screenObjs.homeMainContainer));
            lv_obj_set_size(tutorialHighlightHome,
                lv_obj_get_width(screenObjs.homeMainContainer),
                lv_obj_get_height(screenObjs.homeMainContainer));
            break;
            
        case tutorialScreenLeftTab:
            // show left tab and hide main container
            lv_obj_clear_flag(tutorialHighlightHome, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(leftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(tutorialHighlightLeftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(tutorialTextLeftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(tutorialTextHome, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(tutorialTextLeftTab, "Use the left tab to navigate\nTouch tab to continue.");
            // hide all other highlights
            lv_obj_add_flag(tutorialHighlightHome, LV_OBJ_FLAG_HIDDEN);

            // Highlight the activity tab
            lv_obj_clear_flag(tutorialHighlightLeftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(tutorialHighlightLeftTab,
                lv_obj_get_x(tabActivity),
                (lv_obj_get_y(tabActivity) - 16));
            lv_obj_set_size(tutorialHighlightLeftTab,
                lv_obj_get_width(tabActivity),
                (lv_obj_get_height(tabActivity) + 10));
            break;

        case tutorialScreenStatusBar: {
            // Hide previous tutorial elements
            lv_obj_add_flag(tutorialTextLeftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(tutorialHighlightLeftTab, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(statusBarObj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(leftTab, LV_OBJ_FLAG_HIDDEN);
            
            // Show status bar tutorial elements
            lv_obj_clear_flag(tutorialTextStatusBar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(tutorialHighlightStatusBar, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(tutorialTextStatusBar, "Status bar icons\nTap the wifi symbol");
            
            // create wifi icon in the main overlay
            tempWifiIcon = lv_img_create(tutorialOverlay);
            lv_img_set_src(tempWifiIcon, "S:/wifi40x40.png");
            lv_obj_set_style_bg_opa(tempWifiIcon, LV_OPA_0, LV_PART_MAIN);
            lv_obj_set_style_img_recolor(tempWifiIcon, theme->primaryColor, LV_PART_MAIN);
            lv_obj_set_style_img_recolor_opa(tempWifiIcon, LV_OPA_COVER, LV_PART_MAIN);
            
            // Position the wifi icon in the top right corner
            lv_obj_set_size(tempWifiIcon, 40, 40);  // Set explicit size
            lv_obj_set_pos(tempWifiIcon, 
                LV_HOR_RES - 48,  // Screen width minus margin and icon width
                8);               // Top margin
            
            lv_obj_add_flag(tempWifiIcon, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_ext_click_area(tempWifiIcon, 48);
            lv_obj_add_event_cb(tempWifiIcon, tutorialStatusBarClickHandler, LV_EVENT_CLICKED, NULL);

            // highlight the wifi icon
            lv_obj_clear_flag(tutorialHighlightStatusBar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(tutorialHighlightStatusBar,
                LV_HOR_RES - 53,  // Match wifi icon position minus padding
                3);               // Match wifi icon position minus padding
            lv_obj_set_size(tutorialHighlightStatusBar,
                50,  // Fixed size to match icon + padding
                50); // Fixed size to match icon + padding
            break;
        }

        case tutorialScreenWebUI:
            // Hide previous tutorial elements
            lv_obj_add_flag(tutorialTextStatusBar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(tutorialHighlightStatusBar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(statusBarObj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(leftTab, LV_OBJ_FLAG_HIDDEN);
            
            // Show WebUI tutorial elements
            lv_obj_clear_flag(tutorialTextWebUI, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(tutorialHighlightWebUI, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(tutorialTextWebUI, "Access the WebUI\nScan the QR code and Tap screen to end tutorial");

                        lv_obj_set_pos(tutorialHighlightWebUI, 
                lv_obj_get_x(screenObjs.settingsMainContainer),
                lv_obj_get_y(screenObjs.settingsMainContainer));
            lv_obj_set_size(tutorialHighlightWebUI,
                lv_obj_get_width(screenObjs.settingsMainContainer),
                lv_obj_get_height(screenObjs.settingsMainContainer));
            break;
        default:
            ESP_LOGE(TAG, "Unknown tutorial screen");
            break;
    }
}


// Function to handle tutorial progression
static void tutorialClickHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Move to next tutorial screen
        currentTutorialScreen = static_cast<tutorialScreens>(currentTutorialScreen + 1);
        updateTutorialContent();
    }
}


static void tutorialHomeClickHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        clockContainerEventCallback(e);
        tutorialClickHandler(e);
    }
}

static void tutorialLeftTabClickHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        switchToScreen(activeScreenActivity);
        tutorialClickHandler(e);
    }
}

// Add this new click handler function
static void tutorialStatusBarClickHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        switchToScreen(activeScreenSettings);
        tutorialClickHandler(e);
    }
}

static void tutorialWebUIClickHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        switchToScreen(activeScreenHome);
        endTutorial();
    }
}

// Function to create the tutorial overlay
static void createTutorialOverlay() {
    uiTheme_t* theme = getCurrentTheme();
    
                lv_obj_add_flag(statusBarObj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(leftTab, LV_OBJ_FLAG_HIDDEN);
            
    // Create semi-transparent overlay
    tutorialOverlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(tutorialOverlay);
    lv_obj_set_size(tutorialOverlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(tutorialOverlay, theme->backgroundColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tutorialOverlay, LV_OPA_0, LV_PART_MAIN);
    
    // Create text container

    tutorialTextHome = lv_label_create(tutorialOverlay);
    lv_obj_set_width(tutorialTextHome, LV_PCT(80));
    lv_obj_align(tutorialTextHome, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_font(tutorialTextHome, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(tutorialTextHome, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(tutorialTextHome, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(tutorialTextHome, LV_OBJ_FLAG_HIDDEN);

    tutorialTextLeftTab = lv_label_create(tutorialOverlay);
    lv_obj_set_width(tutorialTextLeftTab, LV_PCT(80));
    lv_obj_align(tutorialTextLeftTab, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_font(tutorialTextLeftTab, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(tutorialTextLeftTab, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(tutorialTextLeftTab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(tutorialTextLeftTab, LV_OBJ_FLAG_HIDDEN);

    // Create highlight container (initially hidden)
    tutorialHighlightHome = lv_obj_create(tutorialOverlay);
    lv_obj_set_style_bg_opa(tutorialHighlightHome, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_color(tutorialHighlightHome, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(tutorialHighlightHome, 4, LV_PART_MAIN);
    lv_obj_add_flag(tutorialHighlightHome, LV_OBJ_FLAG_HIDDEN);

    tutorialHighlightLeftTab = lv_obj_create(tutorialOverlay);
    lv_obj_set_style_bg_opa(tutorialHighlightLeftTab, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_color(tutorialHighlightLeftTab, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(tutorialHighlightLeftTab, 2, LV_PART_MAIN);
    lv_obj_add_flag(tutorialHighlightLeftTab, LV_OBJ_FLAG_HIDDEN);

    // Create status bar tutorial elements
    tutorialTextStatusBar = lv_label_create(tutorialOverlay);
    lv_obj_set_width(tutorialTextStatusBar, LV_PCT(80));
    lv_obj_align(tutorialTextStatusBar, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_font(tutorialTextStatusBar, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(tutorialTextStatusBar, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(tutorialTextStatusBar, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(tutorialTextStatusBar, LV_OBJ_FLAG_HIDDEN);

    tutorialHighlightStatusBar = lv_obj_create(tutorialOverlay);  // Create in overlay instead of highlight container
    lv_obj_remove_style_all(tutorialHighlightStatusBar);  // Remove all default styles
    lv_obj_set_style_bg_opa(tutorialHighlightStatusBar, LV_OPA_0, LV_PART_MAIN);  // Transparent background
    lv_obj_set_style_border_color(tutorialHighlightStatusBar, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(tutorialHighlightStatusBar, 2, LV_PART_MAIN);
    lv_obj_add_flag(tutorialHighlightStatusBar, LV_OBJ_FLAG_HIDDEN);

    tutorialTextWebUI = lv_label_create(tutorialOverlay);
    lv_obj_set_width(tutorialTextWebUI, LV_PCT(80));
    lv_obj_align(tutorialTextWebUI, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_font(tutorialTextWebUI, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(tutorialTextWebUI, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(tutorialTextWebUI, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(tutorialTextWebUI, LV_OBJ_FLAG_HIDDEN);

    tutorialHighlightWebUI = lv_obj_create(tutorialOverlay);
    lv_obj_set_style_bg_opa(tutorialHighlightWebUI, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_color(tutorialHighlightWebUI, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(tutorialHighlightWebUI, 2, LV_PART_MAIN);
    lv_obj_add_flag(tutorialHighlightWebUI, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(tutorialTextWebUI, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(tutorialHighlightWebUI, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(tutorialHighlightWebUI, tutorialWebUIClickHandler, LV_EVENT_CLICKED, NULL);
}



void tutorialScreen() {
    ESP_LOGI(TAG, "Starting tutorial screen");
    createTutorialOverlay();
    updateTutorialContent();
    
    // Add click handler to the overlay
    //lv_obj_add_event_cb(tutorialOverlay, tutorialClickHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(tutorialHighlightHome, tutorialHomeClickHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(tutorialHighlightLeftTab, tutorialLeftTabClickHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(tutorialHighlightStatusBar, tutorialStatusBarClickHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(tutorialHighlightWebUI, tutorialWebUIClickHandler, LV_EVENT_CLICKED, NULL);

}

void switchToTutorialScreen(tutorialScreens newScreen) {
    currentTutorialScreen = newScreen;
    updateTutorialContent();
}

void endTutorial() {
    saveSettingsToNVSasU16(NVS_KEY_TUTORIAL_COMPLETED, 1);
    lv_obj_add_flag(tutorialOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(statusBarObj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(leftTab, LV_OBJ_FLAG_HIDDEN);
}
