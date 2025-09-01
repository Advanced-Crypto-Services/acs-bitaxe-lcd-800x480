#include <Arduino.h>
#include "homeScreen.h"
#include "UIScreens.h"
#include "UIThemes.h"
#include <TimeLib.h>
#include "Clock.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include "mempoolAPI.h"



static bool clockContainerShown = true;
static bool blockClockContainerShown = false;
static bool btcPriceContainerShown = false;
static bool hashrateContainerShown = false;
bool enableClockScreen = true;
bool enableBlockScreen = true;
bool enableBtcPriceScreen = true;
bool enableHashrateScreen = true;

lv_timer_t* clockUpdateTimer = nullptr;

static lv_obj_t* clockLabel = nullptr;
static lv_obj_t* dateLabel = nullptr;
static lv_obj_t* btcPriceLabel = nullptr;
static lv_obj_t* blockClockLabel = nullptr;
static lv_obj_t* halvingLabel = nullptr;
static lv_obj_t* btcConversionLabel = nullptr;
static lv_obj_t* hashrateLabel = nullptr;
static lv_obj_t* hashrateUnitLabel = nullptr;

// This is used because Timelib day function is having issues
static const char* customDayStr(uint8_t day)
{ 
    static const char* const days[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (day > 7) return "";
    return days[day];
}

// Returns the next enabled screen index (0: clock, 1: block, 2: btc price, 3: hashrate)
static int getNextEnabledScreen(int current) {
    for (int i = 1; i <= 4; ++i) { // Try next 4 screens (wrap around)
        int next = (current + i) % 4;
        if ((next == 0 && enableClockScreen) ||
            (next == 1 && enableBlockScreen) ||
            (next == 2 && enableBtcPriceScreen) ||
            (next == 3 && enableHashrateScreen)) {
            return next;
        }
    }
    return current; // fallback, should not happen
}

void clockContainerEventCallback(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE))
    {
        // Find current screen index
        int currentScreen = 0;
        if (blockClockContainerShown) currentScreen = 1;
        else if (btcPriceContainerShown) currentScreen = 2;
        else if (hashrateContainerShown) currentScreen = 3;

        // Get next enabled screen
        int nextScreen = getNextEnabledScreen(currentScreen);

        // Reset all
        clockContainerShown = false;
        blockClockContainerShown = false;
        btcPriceContainerShown = false;
        hashrateContainerShown = false;

        // Set the next screen
        if (nextScreen == 0) clockContainerShown = true;
        else if (nextScreen == 1) blockClockContainerShown = true;
        else if (nextScreen == 2) btcPriceContainerShown = true;
        else if (nextScreen == 3) hashrateContainerShown = true;

        // Update visibility
        lvgl_port_lock(10);
        lv_obj_add_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btcPriceLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btcConversionLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(hashrateLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(hashrateUnitLabel, LV_OBJ_FLAG_HIDDEN);

        if (clockContainerShown) {
            lv_obj_clear_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
        } else if (blockClockContainerShown) {
            lv_obj_clear_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
        } else if (btcPriceContainerShown) {
            lv_obj_clear_flag(btcPriceLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(btcConversionLabel, LV_OBJ_FLAG_HIDDEN);
        } else if (hashrateContainerShown) {
            lv_obj_clear_flag(hashrateLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(hashrateUnitLabel, LV_OBJ_FLAG_HIDDEN);
        }
        lvgl_port_unlock();
    }
}

void homeScreen() 
{
    uiTheme_t* theme = getCurrentTheme();

    // Create the clock container
    lv_obj_t* clockContainer = lv_obj_create(screenObjs.homeMainContainer);
    lv_obj_set_size(clockContainer, 672, 304);
    lv_obj_align(clockContainer, LV_ALIGN_CENTER, 0, -48);
    lv_obj_set_style_bg_opa(clockContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(clockContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(clockContainer, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_border_width(clockContainer, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(clockContainer, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_add_flag(clockContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(clockContainer, clockContainerEventCallback, LV_EVENT_CLICKED, NULL);
    
    // Create the clock label
    clockLabel = lv_label_create(clockContainer);
    lv_label_set_text(clockLabel, "-- : --");
    lv_obj_set_style_text_font(clockLabel, theme->fontExtraBold144, LV_PART_MAIN);
    lv_obj_set_style_text_color(clockLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(clockLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(clockLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    //lv_obj_set_style_border_width(clockLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(clockLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);

    // Create the date label
    dateLabel = lv_label_create(clockContainer);
    lv_label_set_text(dateLabel, "Syncing . . .");
    lv_obj_set_style_text_font(dateLabel, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(dateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(dateLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(dateLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(dateLabel, LV_ALIGN_BOTTOM_MID, 0, 16);
    //lv_obj_set_style_border_width(dateLabel, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(dateLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);

    // create block clock in clock container, hidden by default
    blockClockLabel = lv_label_create(clockContainer);
    lv_label_set_text(blockClockLabel, "Block Height:\n --");
    lv_obj_set_style_text_font(blockClockLabel, theme->fontExtraBold72, LV_PART_MAIN);
    lv_obj_set_style_text_color(blockClockLabel, theme->textColor, LV_PART_MAIN);

    lv_obj_align(blockClockLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(blockClockLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_set_style_border_width(clockLabel, 1, LV_PART_MAIN);

    //lv_obj_set_style_border_color(clockLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);

    // Create the halving label
    halvingLabel = lv_label_create(clockContainer);
    lv_label_set_text(halvingLabel, "Syncing . . .");
    lv_obj_set_style_text_font(halvingLabel, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(halvingLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_align(halvingLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(halvingLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(halvingLabel, LV_ALIGN_BOTTOM_MID, 0, 24);
    lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_set_style_border_width(dateLabel, 1, LV_PART_MAIN);

    // Create the btc price label
    btcPriceLabel = lv_label_create(clockContainer);
    lv_label_set_text(btcPriceLabel, "$--");
    lv_obj_set_style_text_font(btcPriceLabel, theme->fontExtraBold144, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcPriceLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcPriceLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(btcPriceLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(btcPriceLabel, LV_OBJ_FLAG_HIDDEN);

    // Create the btc conversion label
    btcConversionLabel = lv_label_create(clockContainer);
    lv_label_set_text(btcConversionLabel, "BTC/USD");
    lv_obj_set_style_text_font(btcConversionLabel, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcConversionLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(btcConversionLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align_to(btcConversionLabel,btcPriceLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_obj_set_style_text_align(btcConversionLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(btcConversionLabel, LV_OBJ_FLAG_HIDDEN);

    // Create the hashrate label
    hashrateLabel = lv_label_create(clockContainer);
    lv_label_set_text(hashrateLabel, "--");
    lv_obj_set_style_text_font(hashrateLabel, theme->fontExtraBold144, LV_PART_MAIN);
    lv_obj_set_style_text_color(hashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(hashrateLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(hashrateLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(hashrateLabel, LV_OBJ_FLAG_HIDDEN);

    // Create the hashrate unit label
    hashrateUnitLabel = lv_label_create(clockContainer);
    lv_label_set_text(hashrateUnitLabel, "HASHRATE");
    lv_obj_set_style_text_font(hashrateUnitLabel, theme->fontExtraBold32, LV_PART_MAIN);
    lv_obj_set_style_text_color(hashrateUnitLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_set_style_text_opa(hashrateUnitLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align_to(hashrateUnitLabel, hashrateLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_obj_set_style_text_align(hashrateUnitLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(hashrateUnitLabel, LV_OBJ_FLAG_HIDDEN);

    // Create the clock timer
    screenObjs.clockTimer = lv_timer_create([](lv_timer_t* timer) 
    {
        if (clockContainerShown)
        {
            
            // Get values outside the lock
            uint8_t h = hourFormat12();
            uint8_t m = minute();
            uint8_t s = second();
            uint8_t mo = month();
            uint8_t d = day();


        uint8_t w = weekday();
        uint16_t y = year();    
        bool isAm = isAM();

        // Lock for LVGL operations
        if (lvgl_port_lock(10)) 
        {  // 10ms timeout
            //lv_obj_clear_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_clear_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            


            // Check if the time is before 2000, This is used to check if the time has been set
            if (now() < 946684800) 
            {
                Serial.println("Time is before 2000");
                lv_label_set_text(dateLabel, "Syncing . . .");
            }
            else 
            {
                // Batch all LVGL operations together
                lv_label_set_text_fmt(clockLabel, "%2d:%02d%s", h, m, isAm ? "AM" : "PM");
                lv_label_set_text_fmt(dateLabel, "%s\n%s %d %d", customDayStr(w), monthStr(mo), d, y);
            }
            
            lvgl_port_unlock();
        }
        }
        else if (blockClockContainerShown)
        {
            


            // Get values outside the lock
            MempoolApiState* mempoolState = getMempoolState();
            uint32_t blockHeight = mempoolState->blockHeight;
            uint32_t blockToHalving = 1050000 - blockHeight;


        // Lock for LVGL operations
        if (lvgl_port_lock(10)) 
        {  // 10ms timeout
            //lv_obj_clear_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
           // lv_obj_clear_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            //lv_obj_add_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(blockClockLabel, "Block Height:\n %lu", blockHeight);
            lv_label_set_text_fmt(halvingLabel, "Halving In:\n %lu Blocks", blockToHalving);
            

            lvgl_port_unlock();

        }
        }
        else if (btcPriceContainerShown)
        {
            // Get the btc price
            MempoolApiState* mempoolState = getMempoolState();
            uint32_t btcPrice = mempoolState->priceUSD;
            lv_label_set_text_fmt(btcPriceLabel, "$%d,%03d", (int)(btcPrice/1000), (int)(btcPrice) % 1000);
        }
        else if (hashrateContainerShown)
        {
            // Get the hashrate from mining data
            float hashrate = IncomingData.mining.hashrate;

            // Lock for LVGL operations
            if (lvgl_port_lock(10)) 
            {  // 10ms timeout
                // Format hashrate with appropriate units
                if (hashrate >= 1000) {
                    lv_label_set_text_fmt(hashrateLabel, "%d.%02dTH/s", (int)(hashrate / 1000), (int)((hashrate / 1000 - (int)(hashrate / 1000)) * 100));
                } else {
                    lv_label_set_text_fmt(hashrateLabel, "%dGH/s", (int)hashrate);
                }
                lvgl_port_unlock();
            }
        }
    }, 1000, &clockLabel);
}







