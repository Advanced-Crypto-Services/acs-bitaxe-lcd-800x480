#include <Arduino.h>
#include "homeScreen.h"
#include "UIScreens.h"
#include "UIThemes.h"
#include <TimeLib.h>
#include "Clock.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include "mempoolAPI.h"

static void updateLabels(lv_timer_t* timer) 
{

    
    // Get values outside the lock
    char ssid[32], ip[32], status[32], pool[64];
    float hashrate = IncomingData.mining.hashrate;
    float efficiency = IncomingData.mining.efficiency;
    uint32_t shares = IncomingData.mining.shares;
    float temp = IncomingData.monitoring.temperatures[0];
    uint32_t acceptedShares = IncomingData.mining.acceptedShares;
    uint32_t rejectedShares = IncomingData.mining.rejectedShares;
    float rejectRatePercent = IncomingData.mining.rejectRatePercent;
    uint32_t fanSpeed = IncomingData.monitoring.fanSpeed;
    uint32_t asicFreq = IncomingData.monitoring.asicFrequency;
    uint32_t uptime = IncomingData.monitoring.uptime;
    float voltage = IncomingData.monitoring.powerStats.voltage;
    float power = IncomingData.monitoring.powerStats.power;
    float current = (voltage != 0) ? (power * 1000) / voltage : 0;
    float domainVoltage = IncomingData.monitoring.powerStats.domainVoltage;
    
    // Lock for LVGL operations
    if (lvgl_port_lock(10)) 
    {  // 10ms timeout
        lv_obj_t** labels = (lv_obj_t**)timer->user_data;
        
        // Batch all LVGL operations together
        lv_label_set_text_fmt(labels[0], "SSID: %s", IncomingData.network.ssid);
        lv_label_set_text_fmt(labels[1], "IP: %s", IncomingData.network.ipAddress);
        lv_label_set_text_fmt(labels[2], "Status: %s", IncomingData.network.wifiStatus);
        lv_label_set_text_fmt(labels[3], "Pool 1: %s:%d\nPool 2: %s:%d", IncomingData.network.poolUrl, IncomingData.network.poolPort, IncomingData.network.fallbackUrl, IncomingData.network.fallbackPort);
        lv_label_set_text_fmt(labels[4], "Hashrate: %d.%02d GH/s", (int)hashrate, (int)(hashrate * 100) % 100);
        lv_label_set_text_fmt(labels[5], "Efficiency: %d.%02d W/TH", (int)efficiency, (int)(efficiency * 100) % 100);
        lv_label_set_text_fmt(labels[6], "Best Diff: %s", IncomingData.mining.bestDiff);
        lv_label_set_text_fmt(labels[7], "Session Diff: %s", IncomingData.mining.sessionDiff);
        lv_label_set_text_fmt(labels[8], "Accepted: %lu", acceptedShares);
        lv_label_set_text_fmt(labels[9], "Rejected: %lu", rejectedShares);
        lv_label_set_text_fmt(labels[10], "Temp: %d°C", (int)temp);
        lv_label_set_text_fmt(labels[11], "Fan: %lu RPM", fanSpeed);
        lv_label_set_text_fmt(labels[12], "Fan: %d%%", (int)IncomingData.monitoring.fanSpeedPercent);
        lv_label_set_text_fmt(labels[13], "ASIC: %lu MHz", asicFreq);
        lv_label_set_text_fmt(labels[14], "Uptime: %lu:%02lu:%02lu", (uptime / 3600), (uptime % 3600) / 60, uptime % 60);
        lv_label_set_text_fmt(labels[15], "Voltage: %d.%02d V", (int)(voltage / 1000), (int)(voltage / 10) % 100);
       // lv_label_set_text_fmt(labels[16], "Current: %d.%02d A", (int)(current / 1000), (int)(current / 10) % 100); // the TSP546 has incorrect current reading
        lv_label_set_text_fmt(labels[16], "Current: %d.%02d A", (int)current, (int)(current * 100) % 100);
        lv_label_set_text_fmt(labels[17], "Power: %d.%02d W", (int)power, (int)(power * 100) % 100);
        lv_label_set_text_fmt(labels[18], "Domain: %d mV", (int)domainVoltage);
        
        lvgl_port_unlock();
    }
}

void activityScreen() 
{
    uiTheme_t* theme = getCurrentTheme();
    activeScreen = activeScreenActivity;
    //lvgl_port_lock(-1);

    //This is used for Gradient 
    /*static lv_opa_t maskMap[MASK_WIDTH * MASK_HEIGHT];
    // Create canvas for mask
    lv_obj_t * canvas = lv_canvas_create(screenObjs.activityMainContainer);
    lv_canvas_set_buffer(canvas, maskMap, MASK_WIDTH, MASK_HEIGHT, LV_IMG_CF_ALPHA_8BIT);
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_TRANSP);

    // Draw text to canvas
    lv_draw_label_dsc_t labelDsc;
    lv_draw_label_dsc_init(&labelDsc);
    labelDsc.color = lv_color_white();
    labelDsc.font = &interExtraBold56;
    labelDsc.align = LV_TEXT_ALIGN_CENTER;
    //lv_canvas_draw_text(canvas, 5, 5, MASK_WIDTH, &labelDsc, "10 : 00AM");

    // Delete canvas after creating mask
    lv_obj_del(canvas);
    */ 

    // Not using Logos on the activity screen at the moment.
    /*
    lv_obj_t* logos = lv_img_create(screenObjs.activityMainContainer);
    lv_img_set_src(logos, "S:/Logos.png");
    lv_obj_align(logos, LV_ALIGN_TOP_MID, 0, 0);
    lv_img_set_zoom(logos, 384);
    */

    /*
    // Create gradient object
    lv_obj_t * gradObj = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(gradObj, MASK_WIDTH, MASK_HEIGHT);
    lv_obj_center(gradObj);
    lv_obj_set_style_bg_color(gradObj, lv_color_hex(0x34D399), 0);
    lv_obj_set_style_bg_grad_color(gradObj, lv_color_hex(0xA7F3D0), 0);
    lv_obj_set_style_border_width(gradObj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(gradObj, LV_OPA_0, LV_PART_MAIN);
    //lv_obj_set_style_bg_grad_dir(gradObj, LV_GRAD_DIR_VER, 0);
    //lv_obj_add_event_cb(gradObj, addMaskEventCallback, LV_EVENT_ALL, maskMap);
    */

    // Create container for network info
    lv_obj_t* networkContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(networkContainer, 360, 184);
    lv_obj_align(networkContainer, LV_ALIGN_TOP_LEFT, -24, 0);
    lv_obj_set_style_bg_opa(networkContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(networkContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(networkContainer, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_border_width(networkContainer, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(networkContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    Serial.println("Network Container Created");

    //Network Container Label
    lv_obj_t* networkContainerLabel = lv_label_create(networkContainer);
    lv_label_set_text(networkContainerLabel, "Network");
    lv_obj_set_style_text_font(networkContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(networkContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(networkContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Network Container Label Created");
    // SSID Label
    lv_obj_t* ssidLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(ssidLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ssidLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(ssidLabel, "SSID: %s", IncomingData.network.ssid);
    lv_obj_align(ssidLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("SSID Label Created");
    // IP Address Label
    lv_obj_t* ipLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(ipLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ipLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(ipLabel, "IP: %s", IncomingData.network.ipAddress);
    lv_obj_align(ipLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("IP Address Label Created");
    // WiFi Status Label
    lv_obj_t* wifiStatusLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(wifiStatusLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifiStatusLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(wifiStatusLabel, "Status: %s", IncomingData.network.wifiStatus);
    lv_obj_align(wifiStatusLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("WiFi Status Label Created");
    // Pool Info Label
    lv_obj_t* poolUrlLabel = lv_label_create(networkContainer);
    lv_obj_set_style_text_font(poolUrlLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(poolUrlLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(poolUrlLabel, "Pool 1: %s:%d\nPool 2: %s:%d", IncomingData.network.poolUrl, IncomingData.network.poolPort, IncomingData.network.fallbackUrl, IncomingData.network.fallbackPort);
    lv_obj_align(poolUrlLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("Pool Info Label Created");

   

    // Create container for mining info
    lv_obj_t* miningContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(miningContainer, 304, 184);
    lv_obj_align(miningContainer, LV_ALIGN_TOP_RIGHT, 16, 0);
    lv_obj_set_style_bg_opa(miningContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(miningContainer, 0, LV_PART_MAIN);
    //lv_obj_set_style_border_width(miningContainer, 1, LV_PART_MAIN);
    //lv_obj_set_style_border_color(miningContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_obj_clear_flag(miningContainer, LV_OBJ_FLAG_SCROLLABLE);
    Serial.println("Mining Container Created");

    // Mining Container Label
    lv_obj_t* miningContainerLabel = lv_label_create(miningContainer);
    lv_label_set_text(miningContainerLabel, "Mining");
    lv_obj_set_style_text_font(miningContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(miningContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(miningContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Mining Container Label Created");
    // Hashrate Label
    lv_obj_t* hashrateLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(hashrateLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(hashrateLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(hashrateLabel, "Hashrate: %d.%02d GH/s", (int)IncomingData.mining.hashrate, (int)(IncomingData.mining.hashrate * 100) % 100);
    lv_obj_align(hashrateLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("Hashrate Label Created");
    // Efficiency Label
    lv_obj_t* efficiencyLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(efficiencyLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(efficiencyLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(efficiencyLabel, "Efficiency: %d.%02d W/TH", (int)IncomingData.mining.efficiency, (int)(IncomingData.mining.efficiency * 100) % 100);
    lv_obj_align(efficiencyLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("Efficiency Label Created");
    // Best Difficulty Label
    lv_obj_t* bestDiffLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(bestDiffLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(bestDiffLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(bestDiffLabel, "Best Diff: %s", IncomingData.mining.bestDiff);
    lv_obj_align(bestDiffLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("Best Difficulty Label Created");
    // Session Difficulty Label
    lv_obj_t* sessionDiffLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(sessionDiffLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(sessionDiffLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(sessionDiffLabel, "Session Diff: %s", IncomingData.mining.sessionDiff);
    lv_obj_align(sessionDiffLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("Session Difficulty Label Created");

    /*// Shares Label
    lv_obj_t* sharesLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(sharesLabel, &interMedium16_19px, LV_PART_MAIN);
    lv_obj_set_style_text_color(sharesLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_label_set_text_fmt(sharesLabel, "Shares: %u", IncomingData.mining.shares);
    lv_obj_align(sharesLabel, LV_ALIGN_TOP_LEFT, 16, 104);
    Serial.println("Shares Label Created");*/
   
    //accepted shares label
    lv_obj_t* acceptedSharesLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(acceptedSharesLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(acceptedSharesLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(acceptedSharesLabel, "Accepted Shares: %u", IncomingData.mining.acceptedShares);
    lv_obj_align(acceptedSharesLabel, LV_ALIGN_TOP_LEFT, 16, 104);
    Serial.println("Accepted Shares Label Created");
    //rejected shares label
    lv_obj_t* rejectedSharesLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(rejectedSharesLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(rejectedSharesLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(rejectedSharesLabel, "Rejected Shares: %u", IncomingData.mining.rejectedShares);
    lv_obj_align(rejectedSharesLabel, LV_ALIGN_TOP_LEFT, 16, 128);
    Serial.println("Rejected Shares Label Created");

    /*// rejected shares percent label
    lv_obj_t* rejectedSharesPercentLabel = lv_label_create(miningContainer);
    lv_obj_set_style_text_font(rejectedSharesPercentLabel, &interMedium16_19px, LV_PART_MAIN);
    lv_obj_set_style_text_color(rejectedSharesPercentLabel, lv_color_hex(0xA7F3D0), LV_PART_MAIN);
    lv_label_set_text_fmt(rejectedSharesPercentLabel, "Reject Rate: %d.%02d%%", (int)IncomingData.mining.rejectRatePercent, (int)(IncomingData.mining.rejectRatePercent * 100) % 100);
    lv_obj_align(rejectedSharesPercentLabel, LV_ALIGN_TOP_LEFT, 16, 176);
    Serial.println("Rejected Shares Percent Label Created");*/


    // Monitoring Container 
    lv_obj_t* monitoringContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(monitoringContainer, 304, 152);
    lv_obj_align(monitoringContainer, LV_ALIGN_BOTTOM_RIGHT, 16, 0);
    Serial.println("Monitoring Container Created");
    lv_obj_set_style_bg_opa(monitoringContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(monitoringContainer, 0, LV_PART_MAIN);
    lv_obj_clear_flag(monitoringContainer, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_border_color(monitoringContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);

    // Monitoring Container Label
    lv_obj_t* monitoringContainerLabel = lv_label_create(monitoringContainer);
    lv_label_set_text(monitoringContainerLabel, "Monitoring");
    lv_obj_set_style_text_font(monitoringContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(monitoringContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(monitoringContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Monitoring Container Label Created");
    // Temperature Label
    lv_obj_t* tempLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(tempLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(tempLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(tempLabel, "Temp: %d°C", (int)IncomingData.monitoring.temperatures[0]);
    lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("Temperature Label Created");
    // Fan Speed Label
    lv_obj_t* fanSpeedLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(fanSpeedLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(fanSpeedLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(fanSpeedLabel, "Fan: %f RPM", IncomingData.monitoring.fanSpeed);
    lv_obj_align(fanSpeedLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("Fan Speed Label Created");
    // Fan Speed Percent Label
    lv_obj_t* fanSpeedPercentLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(fanSpeedPercentLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(fanSpeedPercentLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(fanSpeedPercentLabel, "Fan: %d%%", (int)IncomingData.monitoring.fanSpeedPercent);
    lv_obj_align(fanSpeedPercentLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("Fan Speed Percent Label Created");
    // asic Frequency Label
    lv_obj_t* asicFreqLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(asicFreqLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(asicFreqLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(asicFreqLabel, "ASIC: %lu MHz", IncomingData.monitoring.asicFrequency);
    lv_obj_align(asicFreqLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("ASIC Frequency Label Created");
    // uptime label
    lv_obj_t* uptimeLabel = lv_label_create(monitoringContainer);
    lv_obj_set_style_text_font(uptimeLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(uptimeLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(uptimeLabel, "Uptime: %lu:%02lu:%02lu", (IncomingData.monitoring.uptime / 3600), (IncomingData.monitoring.uptime % 3600) / 60, IncomingData.monitoring.uptime % 60);
    lv_obj_align(uptimeLabel, LV_ALIGN_TOP_LEFT, 16, 104);
    Serial.println("Uptime Label Created");


    // Power Container
    lv_obj_t* powerContainer = lv_obj_create(screenObjs.activityMainContainer);
    lv_obj_set_size(powerContainer, 360, 152);
    lv_obj_align(powerContainer, LV_ALIGN_BOTTOM_LEFT, -24, 0);
    lv_obj_set_style_bg_opa(powerContainer, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(powerContainer, 0, LV_PART_MAIN);
    //lv_obj_set_style_border_color(powerContainer, lv_color_hex(0xA7F3D0), LV_PART_MAIN);    
    Serial.println("Power Container Created");
    // Power Container Label
    lv_obj_t* powerContainerLabel = lv_label_create(powerContainer);
    lv_label_set_text(powerContainerLabel, "Power");
    lv_obj_set_style_text_font(powerContainerLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(powerContainerLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(powerContainerLabel, LV_ALIGN_TOP_LEFT, 0, -24);
    Serial.println("Power Container Label Created");
    // voltage label
    lv_obj_t* voltageLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(voltageLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(voltageLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(voltageLabel, "Voltage: %d.%02d V", (int)(IncomingData.monitoring.powerStats.voltage / 1000), (int)(IncomingData.monitoring.powerStats.voltage / 10) % 100);
    lv_obj_align(voltageLabel, LV_ALIGN_TOP_LEFT, 16, 8);
    Serial.println("Voltage Label Created");
    // current label
    lv_obj_t* currentLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(currentLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(currentLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(currentLabel, "Current: 0 A");
    lv_obj_align(currentLabel, LV_ALIGN_TOP_LEFT, 16, 32);
    Serial.println("Current Label Created");
    // Power Label
    lv_obj_t* powerLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(powerLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(powerLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(powerLabel, "Power: %d.%02d W", (int)IncomingData.monitoring.powerStats.power, (int)(IncomingData.monitoring.powerStats.power * 100) % 100);
    lv_obj_align(powerLabel, LV_ALIGN_TOP_LEFT, 16, 56);
    Serial.println("Power Label Created");
    // Domain Voltage Label
    lv_obj_t* domainVoltageLabel = lv_label_create(powerContainer);
    lv_obj_set_style_text_font(domainVoltageLabel, theme->fontMedium16, LV_PART_MAIN);
    lv_obj_set_style_text_color(domainVoltageLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(domainVoltageLabel, "Asic Voltage: %d mV", (int)IncomingData.monitoring.powerStats.domainVoltage);
    lv_obj_align(domainVoltageLabel, LV_ALIGN_TOP_LEFT, 16, 80);
    Serial.println("Domain Voltage Label Created");
    



     static lv_obj_t* allLabels[19] = 
    {
        ssidLabel, ipLabel, wifiStatusLabel, poolUrlLabel,
        hashrateLabel, efficiencyLabel, 
        bestDiffLabel, sessionDiffLabel, acceptedSharesLabel, rejectedSharesLabel,
        tempLabel, fanSpeedLabel, fanSpeedPercentLabel, asicFreqLabel, uptimeLabel, voltageLabel, currentLabel, powerLabel, domainVoltageLabel
    };
    screenObjs.labelUpdateTimer = lv_timer_create(updateLabels, 2000, allLabels);

    //lv_obj_align(mainContainerLabel, LV_ALIGN_CENTER, 0, 0);  // Initial alignment 
    //lv_obj_add_style(mainContainerLabel, &mainContainerBorder, 0);
   // lvgl_port_unlock();


}