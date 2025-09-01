#pragma once

#include "lvgl_port_v8.h"
#include <lvgl.h>
#include <Arduino.h>

extern void lvglTabIcons(lv_obj_t* parent);
extern void backgroundImage(lv_obj_t* parent);
extern void statusBar(lv_obj_t* parent);
extern void mainContainer(lv_obj_t* parent);

extern lv_style_t mainContainerBorder;
extern lv_obj_t* mainContainerObj;

extern lv_timer_t* statusBarTimer;

extern lv_obj_t* statusBarObj;
extern lv_obj_t* leftTab;

extern lv_obj_t* wifiIcon;
extern lv_obj_t* tempIcon;
extern lv_obj_t* cpuIcon;
extern lv_obj_t* pwrIcon;
extern lv_obj_t* btcPrice;


