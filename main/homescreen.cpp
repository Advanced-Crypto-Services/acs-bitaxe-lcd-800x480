#include <Arduino.h>
#include "homeScreen.h"
#include "UIScreens.h"
#include "UIThemes.h"
#include <TimeLib.h>
#include "Clock.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include "mempoolAPI.h"

static lv_obj_t* blockClockLabel;
static lv_obj_t* halvingLabel;
static bool clockContainerShown = true;
static bool blockClockContainerShown = false;

lv_timer_t* clockUpdateTimer = nullptr;

lv_obj_t* clockLabel = nullptr;
lv_obj_t* dateLabel = nullptr;

// This is used because Timelib day function is having issues
static const char* customDayStr(uint8_t day)
{ 
    static const char* const days[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (day > 7) return "";
    return days[day];
}

static void clockContainerEventCallback(lv_event_t* e)


{
    lv_obj_t* obj = lv_event_get_target(e);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE))
    {
        Serial.println("Clock Container Clicked");
        clockContainerShown = !clockContainerShown;
        blockClockContainerShown = !blockClockContainerShown;

        if (clockContainerShown && blockClockContainerShown)
        {
            clockContainerShown = true;
            blockClockContainerShown = false;

        }
        else if (!clockContainerShown && !blockClockContainerShown)
        {
            clockContainerShown = true;
            blockClockContainerShown = false;
        }

        if (clockContainerShown)
        {
            lvgl_port_lock(10);
            lv_obj_add_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            lvgl_port_unlock();
        }

        else if (blockClockContainerShown)
        {
            lvgl_port_lock(10);
            lv_obj_clear_flag(blockClockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(halvingLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(clockLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(dateLabel, LV_OBJ_FLAG_HIDDEN);
            lvgl_port_unlock();
        }

            

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


    //lv_obj_set_style_border_color(dateLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);


    

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
        

    }, 1000, &clockLabel);
}







