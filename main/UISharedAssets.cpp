#include "UISharedAssets.h"
#include "UIScreens.h"
#include "UIHandler.h"
#include "I2CData.h"
#include "fonts.h"
#include <lvgl.h>
#include "BAP.h"
#include "UIThemes.h"
#include "mempoolAPI.h"
#include <WiFi.h>
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
lv_obj_t* presetIcon = nullptr;
lv_obj_t* presetLabel = nullptr;
lv_obj_t* birghtnessIcon = nullptr;
lv_obj_t* birghtnessLabel = nullptr;

lv_style_t mainContainerBorder;


void lvglTabIcons(lv_obj_t*parent)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* tab = lv_img_create(parent);
    lv_obj_set_size(tab, 128, 400);
    lv_obj_set_style_bg_opa(tab, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(tab, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(tab, lv_color_hex(0xFF0000), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(tab, LV_OPA_50, LV_PART_MAIN);
    lv_obj_align(tab, LV_ALIGN_LEFT_MID, -16, 16);

    tabHome = lv_img_create(tab);
    lv_img_set_src(tabHome, "S:/house.png");
    lv_obj_set_style_img_recolor(tabHome, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabHome, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabHome, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabHome, 597);
    lv_obj_align(tabHome, LV_ALIGN_CENTER, 0,-164);
    lv_obj_add_flag(tabHome, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabHome, 24);
    // Add these lines for debug border
    //lv_obj_set_style_border_width(tabHome, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(tabHome, lv_color_hex(0xFF0000), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(tabHome, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabHome, tabIconEventHandler, LV_EVENT_CLICKED, NULL);
    
    tabSettings = lv_img_create(tab);
    lv_img_set_src(tabSettings, "S:/settings56by56.png");
    lv_obj_set_style_img_recolor(tabSettings, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabSettings, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabSettings, LV_OPA_0, LV_PART_MAIN);
    lv_obj_align(tabSettings, LV_ALIGN_CENTER, 0, 164);
    lv_obj_add_flag(tabSettings, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabSettings, 24);
    lv_obj_add_event_cb(tabSettings, tabIconEventHandler, LV_EVENT_CLICKED, NULL);
    
    tabMining = lv_img_create(tab);
    lv_img_set_src(tabMining, "S:/activity.png");
    lv_obj_set_style_img_recolor(tabMining, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabMining, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabMining, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabMining, 597);
    lv_obj_align(tabMining, LV_ALIGN_CENTER, 0, -56);
    lv_obj_add_flag(tabMining, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabMining, 24);
    // Add these lines for debug border
    //lv_obj_set_style_border_width(tabMining, 2, LV_PART_MAIN);
    //lv_obj_set_style_border_color(tabMining, lv_color_hex(0xFF0000), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(tabMining, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabMining, tabIconEventHandler, LV_EVENT_CLICKED, NULL);
    
    tabActivity = lv_img_create(tab);
    lv_img_set_src(tabActivity, "S:/pickaxe.png");
    lv_obj_set_style_img_recolor(tabActivity, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabActivity, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabActivity, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabActivity, 597);
    lv_obj_align(tabActivity, LV_ALIGN_CENTER, 0, 56);
    lv_obj_add_flag(tabActivity, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(tabActivity, 24);
    // Add these lines for debug border
   // lv_obj_set_style_border_width(tabActivity, 2, LV_PART_MAIN);
   // lv_obj_set_style_border_color(tabActivity, lv_color_hex(0xFF0000), LV_PART_MAIN);
  //  lv_obj_set_style_border_opa(tabActivity, LV_OPA_50, LV_PART_MAIN);
    lv_obj_add_event_cb(tabActivity, tabIconEventHandler, LV_EVENT_CLICKED, NULL);

    
    tabBitcoinNews = lv_img_create(tab);
    lv_img_set_src(tabBitcoinNews, "S:/bitcoin.png");
    lv_obj_set_style_img_recolor(tabBitcoinNews, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tabBitcoinNews, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabBitcoinNews, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(tabBitcoinNews, 597);
    lv_obj_align(tabBitcoinNews, LV_ALIGN_CENTER, 0, -164);
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
    lv_obj_set_size(statusBarObj, 720, 64);
    lv_obj_align(statusBarObj, LV_ALIGN_TOP_RIGHT, 0, 8);
    lv_obj_set_style_bg_opa(statusBarObj, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBar, 4, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBar, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    //lv_obj_set_style_border_opa(statusBar, LV_OPA_50, LV_PART_MAIN);

    wifiIcon = lv_img_create(statusBarObj);
    // handle this through the startupDone flag in the timer
    lv_img_set_src(wifiIcon, "S:/wifi40x40.png");
    lv_obj_set_style_bg_opa(wifiIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(wifiIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(wifiIcon, LV_OPA_COVER, LV_PART_MAIN);
    //lv_img_set_zoom(wifiIcon, 512);
    lv_obj_align(wifiIcon, LV_ALIGN_TOP_RIGHT, -8, 0);
    lv_obj_add_flag(wifiIcon, LV_OBJ_FLAG_HIDDEN);


     tempIcon = lv_img_create(statusBarObj);
    lv_img_set_src(tempIcon, "S:/temp40x40.png");
    lv_obj_align(tempIcon, LV_ALIGN_TOP_LEFT, 392, 0);
    lv_obj_set_style_bg_opa(tempIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(tempIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tempIcon, LV_OPA_COVER, LV_PART_MAIN);


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


    cpuIcon = lv_img_create(statusBarObj);
    lv_img_set_src(cpuIcon, "S:/cpu40x40.png");
    lv_obj_set_style_img_recolor(cpuIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cpuIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(cpuIcon, LV_OPA_COVER, LV_PART_MAIN);  
    //lv_img_set_zoom(cpuIcon, 512);
    lv_obj_align(cpuIcon, LV_ALIGN_TOP_LEFT, 256, 0);

    statusBarHashrateLabel = lv_label_create(statusBarObj);
    lv_label_set_text_fmt(statusBarHashrateLabel, "%d GH/s\n%d W/TH", (int)IncomingData.mining.hashrate, (int)IncomingData.mining.efficiency);
    lv_obj_set_style_text_font(statusBarHashrateLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarHashrateLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarHashrateLabel, 96);
    lv_label_set_long_mode(statusBarHashrateLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(statusBarHashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarHashrateLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarHashrateLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarHashrateLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_align_to(statusBarHashrateLabel, cpuIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);

    pwrIcon = lv_img_create(statusBarObj);
    lv_img_set_src(pwrIcon, "S:/plugZap40x40.png");
    lv_obj_set_style_img_recolor(pwrIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(pwrIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pwrIcon, LV_OPA_0, LV_PART_MAIN);
    //lv_img_set_zoom(pwrIcon, 512);
    lv_obj_align(pwrIcon, LV_ALIGN_TOP_LEFT, 128, 0);

    statusBarPowerLabel = lv_label_create(statusBarObj);
    lv_label_set_text_fmt(statusBarPowerLabel, "%d.%03d V\n%d.%02d W", 
        (int)(IncomingData.monitoring.powerStats.domainVoltage/1000), (int)(IncomingData.monitoring.powerStats.domainVoltage) % 1000, 
        (int)(IncomingData.monitoring.powerStats.power), (int)(IncomingData.monitoring.powerStats.power * 100) % 100);
    lv_obj_set_style_text_font(statusBarPowerLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarPowerLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarPowerLabel, 88);
    lv_label_set_long_mode(statusBarPowerLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(statusBarPowerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarPowerLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(statusBarPowerLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(statusBarPowerLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_align_to(statusBarPowerLabel, pwrIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);

    btcPrice = lv_img_create(statusBarObj);
    lv_img_set_src(btcPrice, "S:/bitcoin40x40.png");
    lv_obj_set_style_bg_opa(btcPrice, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(btcPrice, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(btcPrice, LV_OPA_COVER, LV_PART_MAIN);
    //lv_img_set_zoom(btcPrice, 512);
    lv_obj_align(btcPrice, LV_ALIGN_TOP_LEFT, 0, 0);

    statusBarBtcPriceLabel = lv_label_create(statusBarObj);
    lv_label_set_text(statusBarBtcPriceLabel, "$ syncing...");
    lv_obj_set_style_text_align(statusBarBtcPriceLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(statusBarBtcPriceLabel, 144);
    lv_label_set_long_mode(statusBarBtcPriceLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(statusBarBtcPriceLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(statusBarBtcPriceLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(statusBarBtcPriceLabel, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(btcPriceLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(btcPriceLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    //lv_obj_set_style_text_opa(btcPriceLabel, 224, LV_PART_MAIN);
    lv_obj_align_to(statusBarBtcPriceLabel, btcPrice, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);

    presetIcon = lv_img_create(statusBarObj);
    lv_img_set_src(presetIcon, "S:/preset40x40.png");
    lv_obj_set_style_bg_opa(presetIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(presetIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(presetIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(presetIcon, LV_ALIGN_TOP_LEFT, 456, 0);

    presetLabel = lv_label_create(statusBarObj);
    lv_label_set_text(presetLabel, "PRES");
    lv_obj_set_style_text_font(presetLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(presetLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(presetLabel, 80);
    lv_label_set_long_mode(presetLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(presetLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(presetLabel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_align_to(presetLabel, presetIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);

    birghtnessIcon = lv_img_create(statusBarObj);
    lv_img_set_src(birghtnessIcon, "S:/brightness40x40.png");
    lv_obj_set_style_bg_opa(birghtnessIcon, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(birghtnessIcon, theme->primaryColor, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(birghtnessIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(birghtnessIcon, LV_ALIGN_TOP_LEFT, 560, 0);
    
    birghtnessLabel = lv_label_create(statusBarObj);
    lv_label_set_text(birghtnessLabel, "100%");
    lv_obj_set_style_text_font(birghtnessLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_align(birghtnessLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(birghtnessLabel, 80);
    lv_label_set_long_mode(birghtnessLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(birghtnessLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(birghtnessLabel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_align_to(birghtnessLabel, birghtnessIcon, LV_ALIGN_OUT_RIGHT_TOP, 4, 0);
    

    

    screenObjs.statusBarUpdateTimer = lv_timer_create([](lv_timer_t* timer)
    {
        int temp = IncomingData.monitoring.temperatures[0];
        int hashrate = IncomingData.mining.hashrate;
        int efficiency = IncomingData.mining.efficiency;
        float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage;
        float power = IncomingData.monitoring.powerStats.power;
        bool startupDone = specialRegisters.startupDone;

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
    }, 2500, NULL);
}


void mainContainer(lv_obj_t* parent)
{
    uiTheme_t* theme = getCurrentTheme();
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, LV_PART_MAIN);
    static lv_style_t mainContainerBorder;
    lv_style_init(&mainContainerBorder);
    lv_style_set_border_width(&mainContainerBorder, 4);
    lv_style_set_radius(&mainContainerBorder, 36);
    lv_style_set_border_color(&mainContainerBorder, theme->borderColor);
    lv_style_set_border_opa(&mainContainerBorder, LV_OPA_0);
    //lv_style_set_border_side(&mainContainerBorder, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_TOP);
    lv_style_set_bg_opa(&mainContainerBorder, LV_OPA_0);
    lv_style_set_shadow_width(&mainContainerBorder, 56);
    lv_style_set_shadow_color(&mainContainerBorder, theme->borderColor);
    lv_style_set_shadow_opa(&mainContainerBorder, LV_OPA_30);
    lv_style_set_shadow_spread(&mainContainerBorder, 8);
    lv_style_set_shadow_ofs_x(&mainContainerBorder, 0);
    lv_style_set_shadow_ofs_y(&mainContainerBorder, 0);
    lv_obj_add_style(container, &mainContainerBorder, 0);
    lv_obj_set_size(container, 672, 392);
    lv_obj_align(container, LV_ALIGN_CENTER, 40, 28);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
}






