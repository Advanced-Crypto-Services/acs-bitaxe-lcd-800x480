#include "Clock.h"
#include "NVS.h"

int32_t timeOffsetHours = 0;
int32_t timeOffsetMinutes = 0;
char offsetHoursStr[10];
char offsetMinutesStr[10];

void espTime()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial0.println("WiFi not connected - SNTP sync not possible");
        return;
    }
    // Initialize SNTP
    Serial0.println("Initializing SNTP");
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_interval(3600000);  // Sync every hour
    sntp_init();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        Serial0.println("Waiting for SNTP sync...");
        delay(1000);
        time(&now);
        localtime_r(&now, &timeinfo);
        Serial0.printf("Year: %d\n", timeinfo.tm_year);
    }

    loadSettingsFromNVSasString(NVS_KEY_TIME_OFFSET_HOURS, offsetHoursStr, sizeof(offsetHoursStr));
    loadSettingsFromNVSasString(NVS_KEY_TIME_OFFSET_MINUTES, offsetMinutesStr, sizeof(offsetMinutesStr));

    timeOffsetHours = atoi(offsetHoursStr);
    timeOffsetMinutes = atoi(offsetMinutesStr);

    // Set timezone to China Standard Time
    setTimeOffset(timeOffsetHours, timeOffsetMinutes);

    char strftime_buf[64];
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    Serial0.printf("The current date/time in Dallas is: %s\n", strftime_buf);
    IncomingData.status.clockSync = now;
}

uint32_t keepClockTime() 
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    
    // Get current time from ESP's internal time keeping
    time(&now);
    localtime_r(&now, &timeinfo);
    
    // Set TimeLib time using individual components
    setTime(timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec,
            timeinfo.tm_mday,
            timeinfo.tm_mon + 1,    // tm_mon is 0-11, TimeLib expects 1-12
            timeinfo.tm_year + 1900); // tm_year is years since 1900
    
    // For debugging
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    Serial0.printf("Local time: %s\n", strftime_buf);
    
    return now;
}

void setTimeOffset(int32_t offsetHours, int32_t offsetMinutes) {
    timeOffsetHours = offsetHours;
    timeOffsetMinutes = offsetMinutes;
    
    char tz[32];
    int32_t totalOffset = (offsetHours * 3600) + (offsetMinutes * 60);
    
    if (offsetMinutes == 0) {
        // Simple hour-only format
        if (totalOffset >= 0) {
            snprintf(tz, sizeof(tz), "UTC-%ld", offsetHours);
        } else {
            snprintf(tz, sizeof(tz), "UTC+%ld", -offsetHours);
        }
    } else {
        // Format with minutes
        if (totalOffset >= 0) {
            snprintf(tz, sizeof(tz), "UTC-%ld:%02ld", abs(offsetHours), abs(offsetMinutes));
        } else {
            snprintf(tz, sizeof(tz), "UTC+%ld:%02ld", abs(offsetHours), abs(offsetMinutes));
        }
    }
    
    setenv("TZ", tz, 1);
    tzset();



    snprintf(offsetHoursStr, sizeof(offsetHoursStr), "%ld", offsetHours);
    snprintf(offsetMinutesStr, sizeof(offsetMinutesStr), "%ld", offsetMinutes);

    saveSettingsToNVSasString(NVS_KEY_TIME_OFFSET_HOURS, offsetHoursStr, sizeof(offsetHoursStr));
    saveSettingsToNVSasString(NVS_KEY_TIME_OFFSET_MINUTES, offsetMinutesStr, sizeof(offsetMinutesStr));
}

/* Set Time to CDT by default
* 
*/
void setFirstBootTime (){
    setTimeOffset(-5, 0);
}