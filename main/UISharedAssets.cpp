#include "UISharedAssets.h"
#include "UIScreens.h"
#include "UIHandler.h"
#include "fonts.h"
#include <lvgl.h>
#include "BAP.h"
#include "UIThemes.h"
#include "mempoolAPI.h"
#include <WiFi.h>
#include "lcdSettings.h"
lv_timer_t* statusBarTimer = nullptr;
lv_obj_t* statusBarTempLabel = nullptr;
lv_obj_t* statusBarHashrateLabel = nullptr;
lv_obj_t* statusBarPowerLabel = nullptr;
lv_obj_t* statusBarBtcPriceLabel = nullptr;
lv_obj_t* wifiIcon = nullptr;
lv_obj_t* tempIcon = nullptr;
lv_obj_t* cpuIcon = nullptr;
lv_obj_t* pwrIcon = nullptr;
lv_obj_t* btcPrice = nullptr;
lv_obj_t* mainContainerObj = nullptr;
lv_obj_t* birghtnessIcon = nullptr;
lv_obj_t* birghtnessLabel = nullptr;
lv_obj_t* leftTab = nullptr;
lv_style_t mainContainerBorder;

static void statusBarBtcPriceEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        Serial0.println("BTC Price clicked");
        switchToScreen(activeScreenBitcoinNews);
    }
}

static void statusBarPowerEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        Serial0.println("Power clicked");   
        switchToScreen(activeScreenActivity);
    }
}

static void statusBarHashRateEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        Serial0.println("Hash Rate clicked");
        switchToScreen(activeScreenMining);
    }
}

static void statusBarTempEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        Serial0.println("Temp clicked");
        switchToScreen(activeScreenActivity);
    }
}

static void statusBarBrightnessEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        Serial0.println("Brightness clicked");
        switchToScreen(activeScreenSettings);
        lv_tabview_set_act(screenObjs.settingTabView, 1, LV_ANIM_OFF); // 1 is the index of the SCREEN tab
    }
}

static void statusBarWifiEventHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        Serial0.println("Wifi clicked");
        switchToScreen(activeScreenSettings);
        lv_tabview_set_act(screenObjs.settingTabView, 0, LV_ANIM_OFF); // 0 is the index of the WIFI tab
    }
}

void lvglTabIcons(lv_obj_t*parent)
{
    uiTheme_t* theme = getCurrentTheme();
    leftTab = lv_img_create(parent);
    lv_obj_set_size(leftTab, 88, 360);
    lv_obj_set_style_bg_opa(leftTab, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(tab, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(tab, lv_color_hex(0xFF0000), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(tab, LV_OPA_50, LV_PART_MAIN);
    lv_obj_align(leftTab, LV_ALIGN_BOTTOM_LEFT, 0, -30);
    lv_obj_clear_flag(leftTab, LV_OBJ_FLAG_SCROLLABLE);

    tabHome = lv_img_create(leftTab);
    lv_img_set_src(tabHome, "S:/house.png");
    lv_obj_set_style_img_recolor(tabHome, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabHome, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabHome, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabHome, 597);
    lv_obj_align(tabHome, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(tabHome, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabHome, 24);
    // Add these lines for debug border
    //lv_obj_set_style_border_width(tabHome, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(tabHome, lv_color_hex(0xFF0000), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(tabHome, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabHome, tabIconEventHandler, LV_EVENT_CLICKED, NULL);

    tabMining = lv_img_create(leftTab);
    lv_img_set_src(tabMining, "S:/activity.png");
    lv_obj_set_style_img_recolor(tabMining, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabMining, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabMining, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabMining, 597);
    lv_obj_align_to(tabMining, tabHome, LV_ALIGN_OUT_BOTTOM_MID, 0, 46);
    lv_obj_add_flag(tabMining, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabMining, 24);
    // Add these lines for debug border
    //lv_obj_set_style_border_width(tabMining, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(tabMining, lv_color_hex(0xFF0000), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(tabMining, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabMining, tabIconEventHandler, LV_EVENT_CLICKED, NULL);
    
    tabActivity = lv_img_create(leftTab);
    lv_img_set_src(tabActivity, "S:/pickaxe.png");
    lv_obj_set_style_img_recolor(tabActivity, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabActivity, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabActivity, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabActivity, 597);
    lv_obj_align_to(tabActivity, tabMining, LV_ALIGN_OUT_BOTTOM_MID, 0, 46);
    lv_obj_add_flag(tabActivity, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabActivity, 24);
    // Add these lines for debug border
   // lv_obj_set_style_border_width(tabActivity, 2, LV_PART_MAIN);
   // lv_obj_set_style_border_color(tabActivity, lv_color_hex(0xFF0000), LV_PART_MAIN);
  //  lv_obj_set_style_border_opa(tabActivity, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabActivity, tabIconEventHandler, LV_EVENT_CLICKED, NULL);

        
    tabSettings = lv_img_create(leftTab);
    lv_img_set_src(tabSettings, "S:/settings56by56.png");
    lv_obj_set_style_img_recolor(tabSettings, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabSettings, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabSettings, LV_OPA_0, LV_PART_MAIN);
    lv_obj_align_to(tabSettings, tabActivity, LV_ALIGN_OUT_BOTTOM_MID, 0, 46);
    lv_obj_add_flag(tabSettings, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabSettings, 24);
    lv_obj_add_event_cb(tabSettings, tabIconEventHandler, LV_EVENT_CLICKED, NULL);

    
    tabBitcoinNews = lv_img_create(leftTab);
    lv_img_set_src(tabBitcoinNews, "S:/bitcoin.png");
    lv_obj_set_style_img_recolor(tabBitcoinNews, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabBitcoinNews, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabBitcoinNews, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabBitcoinNews, 597);
    lv_obj_align(tabBitcoinNews, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(tabBitcoinNews, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabBitcoinNews, 24);
    // Add these lines for debug border
   // lv_obj_set_style_border_width(tabSettings, 2, LV_PART_MAIN);
   // lv_obj_set_style_border_color(tabSettings, lv_color_hex(0xFF0000), LV_PART_MAIN);
   // lv_obj_set_style_border_opa(tabSettings, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabBitcoinNews, tabIconEventHandler, LV_EVENT_CLICKED, NULL);
}

void backgroundImage(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* background = lv_img_create(parent);
    lv_img_set_src(background, theme->background);
    lv_obj_center(background);
    lv_obj_set_style_bg_opa(background, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(background, theme->backgroundColor, LV_PART_MAIN);
}

void statusBar(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    statusBarObj = lv_img_create(parent);
    lv_obj_set_size(statusBarObj, 800, 49);
    lv_obj_align(statusBarObj, LV_ALIGN_TOP_RIGHT, 0, 8);
    lv_obj_set_style_bg_opa(statusBarObj, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarObj, 4, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarObj, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(statusBarObj, LV_OPA_50, LV_PART_MAIN);

        btcPrice = lv_img_create(statusBarObj);
    lv_img_set_src(btcPrice, "S:/bitcoin40x40.png");
    lv_obj_set_style_bg_opa(btcPrice, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(btcPrice, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(btcPrice, LV_OPA_COVER, LV_PART_MAIN);
    //lv_img_set_zoom(btcPrice, 512);
    lv_obj_align(btcPrice, LV_ALIGN_TOP_LEFT, 32, 0);
    lv_obj_add_flag(btcPrice, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btcPrice, 24);
    lv_obj_add_event_cb(btcPrice, statusBarBtcPriceEventHandler, LV_EVENT_CLICKED, NULL);

    statusBarBtcPriceLabel = lv_label_create(statusBarObj);
    lv_label_set_text(statusBarBtcPriceLabel, "$ syncing...");
    lv_obj_set_style_text_align(statusBarBtcPriceLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarBtcPriceLabel, 112);
    lv_label_set_long_mode(statusBarBtcPriceLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(statusBarBtcPriceLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(statusBarBtcPriceLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarBtcPriceLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarBtcPriceLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarBtcPriceLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_set_style_text_opa(statusBarBtcPriceLabel, 224, LV_PART_MAIN);
    lv_obj_align_to(statusBarBtcPriceLabel, btcPrice, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_flag(statusBarBtcPriceLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(statusBarBtcPriceLabel, 24);
    lv_obj_add_event_cb(statusBarBtcPriceLabel, statusBarBtcPriceEventHandler, LV_EVENT_CLICKED, NULL);


    pwrIcon = lv_img_create(statusBarObj);
    lv_img_set_src(pwrIcon, "S:/plugZap40x40.png");
    lv_obj_set_style_img_recolor(pwrIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(pwrIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pwrIcon, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(pwrIcon, 512);
    lv_obj_align_to(pwrIcon, statusBarBtcPriceLabel, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_flag(pwrIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(pwrIcon, 24);
    lv_obj_add_event_cb(pwrIcon, statusBarPowerEventHandler, LV_EVENT_CLICKED, NULL);


    statusBarPowerLabel = lv_label_create(statusBarObj);
    lv_label_set_text_fmt(statusBarPowerLabel, "%d.%03d V\n%d.%02d W", 
        (int)(IncomingData.monitoring.powerStats.domainVoltage/1000), (int)(IncomingData.monitoring.powerStats.domainVoltage) % 1000, 
        (int)(IncomingData.monitoring.powerStats.power), (int)(IncomingData.monitoring.powerStats.power * 100) % 100);
    lv_obj_set_style_text_font(statusBarPowerLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarPowerLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarPowerLabel, 112);
    lv_label_set_long_mode(statusBarPowerLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(statusBarPowerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarPowerLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarPowerLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarPowerLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_align_to(statusBarPowerLabel, pwrIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);
    lv_obj_add_flag(statusBarPowerLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(statusBarPowerLabel, 24);
    lv_obj_add_event_cb(statusBarPowerLabel, statusBarPowerEventHandler, LV_EVENT_CLICKED, NULL);


    cpuIcon = lv_img_create(statusBarObj);
    lv_img_set_src(cpuIcon, "S:/cpu40x40.png");
    lv_obj_set_style_img_recolor(cpuIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cpuIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(cpuIcon, LV_OPA_COVER, LV_PART_MAIN);  
    //lv_img_set_zoom(cpuIcon, 512);
    lv_obj_align_to(cpuIcon, statusBarPowerLabel, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_flag(cpuIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(cpuIcon, 24);
    lv_obj_add_event_cb(cpuIcon, statusBarHashRateEventHandler, LV_EVENT_CLICKED, NULL);


    statusBarHashrateLabel = lv_label_create(statusBarObj);
    lv_label_set_text_fmt(statusBarHashrateLabel, "%d GH/s\n%d W/TH", (int)IncomingData.mining.hashrate, (int)IncomingData.mining.efficiency);
    lv_obj_set_style_text_font(statusBarHashrateLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarHashrateLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarHashrateLabel, 112);
    lv_label_set_long_mode(statusBarHashrateLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(statusBarHashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarHashrateLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarHashrateLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarHashrateLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_align_to(statusBarHashrateLabel, cpuIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);
    lv_obj_add_flag(statusBarHashrateLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(statusBarHashrateLabel, 24);
    lv_obj_add_event_cb(statusBarHashrateLabel, statusBarHashRateEventHandler, LV_EVENT_CLICKED, NULL);


      tempIcon = lv_img_create(statusBarObj);
    lv_img_set_src(tempIcon, "S:/temp40x40.png");
    lv_obj_align_to(tempIcon, statusBarHashrateLabel, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_set_style_bg_opa(tempIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(tempIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tempIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_flag(tempIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tempIcon, 24);
    lv_obj_add_event_cb(tempIcon, statusBarTempEventHandler, LV_EVENT_CLICKED, NULL);


    statusBarTempLabel = lv_label_create(statusBarObj);
    lv_label_set_text_fmt(statusBarTempLabel, "%d°C", (int)IncomingData.monitoring.temperatures[0]);
    lv_obj_set_style_text_font(statusBarTempLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarTempLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarTempLabel, 80);
    lv_label_set_long_mode(statusBarTempLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(statusBarTempLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarTempLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarTempLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarTempLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_align_to(statusBarTempLabel, tempIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);
    lv_obj_add_flag(statusBarTempLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(statusBarTempLabel, 24);
    lv_obj_add_event_cb(statusBarTempLabel, statusBarTempEventHandler, LV_EVENT_CLICKED, NULL);


    birghtnessIcon = lv_img_create(statusBarObj);
    lv_img_set_src(birghtnessIcon, "S:/brightness40x40.png");
    lv_obj_set_style_bg_opa(birghtnessIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(birghtnessIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(birghtnessIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align_to(birghtnessIcon, statusBarTempLabel, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_flag(birghtnessIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(birghtnessIcon, 24);
    lv_obj_add_event_cb(birghtnessIcon, statusBarBrightnessEventHandler, LV_EVENT_CLICKED, NULL);
    

    birghtnessLabel = lv_label_create(statusBarObj);
    lv_label_set_text_fmt(birghtnessLabel, "%d%%", getBrightness());
    lv_obj_set_style_text_font(birghtnessLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(birghtnessLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(birghtnessLabel, 80);
    lv_label_set_long_mode(birghtnessLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(birghtnessLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(birghtnessLabel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_align_to(birghtnessLabel, birghtnessIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);
    lv_obj_add_flag(birghtnessLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(birghtnessLabel, 24);
    lv_obj_add_event_cb(birghtnessLabel, statusBarBrightnessEventHandler, LV_EVENT_CLICKED, NULL);
    

    wifiIcon = lv_img_create(statusBarObj);
    // handle this through the startupDone flag in the timer
    lv_img_set_src(wifiIcon, "S:/wifi40x40.png");
    lv_obj_set_style_bg_opa(wifiIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(wifiIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(wifiIcon, LV_OPA_COVER, LV_PART_MAIN);
    //lv_img_set_zoom(wifiIcon, 512);
    lv_obj_align(wifiIcon, LV_ALIGN_TOP_RIGHT, -8, 0);
    lv_obj_add_flag(wifiIcon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifiIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(wifiIcon, 48);
    lv_obj_add_event_cb(wifiIcon, statusBarWifiEventHandler, LV_EVENT_CLICKED, NULL);
    

    screenObjs.statusBarUpdateTimer = lv_timer_create([](lv_timer_t* timer)
    {
        int temp = IncomingData.monitoring.temperatures[0];
        int hashrate = IncomingData.mining.hashrate;
        int efficiency = IncomingData.mining.efficiency;
        float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage;
        float power = IncomingData.monitoring.powerStats.power;
        bool startupDone = specialRegisters.startupDone;
        uint8_t brightness = (uint8_t)(getBrightness() * 100 / 255);

        if (lvgl_port_lock(10))
        {
            MempoolApiState* mempoolState = getMempoolState();
            if (mempoolState->priceValid && mempoolState->priceUSD > 0)
            {
                lv_label_set_text_fmt(statusBarBtcPriceLabel, "$%d,%03d", 
                    (int)(mempoolState->priceUSD/1000), 
                    (int)(mempoolState->priceUSD) % 1000);
            }
            else
            {
                lv_label_set_text(statusBarBtcPriceLabel, "$ syncing...");
            }
            if (specialRegisters.overheatMode)
            {
                lv_label_set_text(statusBarTempLabel, "OVER\nHEAT");
                lv_obj_set_style_img_recolor(tempIcon, lv_color_hex(0xFF0000), LV_PART_MAIN);
                lv_obj_set_style_img_recolor_opa(tempIcon, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_text_color(statusBarTempLabel, lv_color_hex(0xFF0000), LV_PART_MAIN); 
            }
            else
            {
                lv_label_set_text_fmt(statusBarTempLabel, "%d°C", temp);
            }
            lv_label_set_text_fmt(statusBarHashrateLabel, "%d GH/s\n%d W/TH", hashrate, efficiency);
            lv_label_set_text_fmt(statusBarPowerLabel, "%d.%03d V\n%d.%02d W", 
                (int)(domainVoltage/1000), (int)(domainVoltage) % 1000, 
                (int)(power), (int)(power * 100) % 100);
            lv_label_set_text_fmt(birghtnessLabel, "%d%%", brightness);
            if (WiFi.status() == WL_CONNECTED)
            {
                lv_obj_clear_flag(wifiIcon, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                lv_obj_add_flag(wifiIcon, LV_OBJ_FLAG_HIDDEN);
            }
            lvgl_port_unlock();
        }
    }, 500, NULL);
}


void mainContainer(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, LV_PART_MAIN);
    static lv_style_t mainContainerBorder;
    lv_style_init(&mainContainerBorder);
    //lv_style_set_border_width(&mainContainerBorder, 4);
    //lv_style_set_border_color(&mainContainerBorder, theme->borderColor);
    lv_style_set_border_opa(&mainContainerBorder, LV_OPA_0);
    //lv_style_set_border_side(&mainContainerBorder, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_TOP);
    lv_obj_add_style(container, &mainContainerBorder, 0);
    lv_obj_set_size(container, 644, 360);
    lv_obj_align(container, LV_ALIGN_CENTER, 22, 30);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
}






