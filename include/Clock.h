#pragma once

#include <Arduino.h>
#include "Time.h"
//#include "DateStrings.h"
#include "BAP.h"
#include "wifiFeatures.h"
#include "httpServer.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_sntp.h"

extern uint32_t keepClockTime();
extern void espTime();
extern int32_t timeOffsetHours;
extern int32_t timeOffsetMinutes;
extern char offsetHoursStr[10];
extern char offsetMinutesStr[10];
extern void setTimeOffset(int32_t offsetHours, int32_t offsetMinutes = 0);