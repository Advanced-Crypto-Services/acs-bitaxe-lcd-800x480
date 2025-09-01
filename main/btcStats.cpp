#include <Arduino.h>
#include "homeScreen.h"
#include "UIScreens.h"
#include "UIThemes.h"
#include <TimeLib.h>
#include "Clock.h"
#include "lvgl_port_v8.h"
#include <lvgl.h>
#include "mempoolAPI.h"

#define btcStats_MAX_VARIATIONS 3  // Number of variations for each container

struct ContainerState {
    lv_obj_t* containers[btcStats_MAX_VARIATIONS];
    int currentIndex;
};

static ContainerState containerStates[4];  // 4 groups of containers

static void updateContainerVisibility(int groupIndex) {
    if (lvgl_port_lock(10)) {
        for (int i = 0; i < btcStats_MAX_VARIATIONS; i++) {
            lv_obj_add_flag(containerStates[groupIndex].containers[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(containerStates[groupIndex].containers[containerStates[groupIndex].currentIndex], LV_OBJ_FLAG_HIDDEN);
        lvgl_port_unlock();
    }
}

static void quadContainerEventCallback(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE)) {
        for (int group = 0; group < 4; group++) {
            for (int i = 0; i < btcStats_MAX_VARIATIONS; i++) {
                if (obj == containerStates[group].containers[i]) {
                    containerStates[group].currentIndex = (containerStates[group].currentIndex + 1) % btcStats_MAX_VARIATIONS;
                    updateContainerVisibility(group);
                    return;
                }
            }
        }
    }
}

static void updateBtcStatsLabels(lv_timer_t* timer) 
{

    MempoolApiState* mempoolState = getMempoolState();
    // Get values outside the lock
    uint32_t btcPrice = mempoolState->priceUSD;
    double networkHashrate = mempoolState->networkHashrate;
    double networkDifficulty = mempoolState->networkDifficulty;
    uint32_t currentFee = mempoolState->fastestFee;
    uint32_t btcPriceEUR = mempoolState->priceEUR;
    uint32_t blockHeight = mempoolState->blockHeight;
    uint32_t currentFeeLow = mempoolState->minimumFee;
    double DifficultyProgressPercent = mempoolState->difficultyProgressPercent;
    double DifficultyChangePercent = mempoolState->difficultyChangePercent;
    // also uses DifficultyProgressPercent
    uint32_t btcPriceCAD = mempoolState->priceCAD; 
    uint32_t halvingCountdown = (1050000 - blockHeight);
    uint32_t averageFee = mempoolState->economyFee;
    // also uses DifficultyProgressPercent
    uint32_t remainingBlocksToDifficultyAdjustment = mempoolState->remainingBlocksToDifficultyAdjustment;
    uint32_t remainingTimeToDifficultyAdjustment = mempoolState->remainingTimeToDifficultyAdjustment;
    // Lock for LVGL operations
    if (lvgl_port_lock(10)) 
    {  // 10ms timeout
        lv_obj_t** labels = (lv_obj_t**)timer->user_data;
        
        // Update btc price label
        lv_label_set_text_fmt(labels[0], "$%d,%03d", (int)(btcPrice/1000), (int)(btcPrice) % 1000);

        // Update network hashrate label
        lv_label_set_text_fmt(labels[1], "%d.%02d EH/s", (int)(networkHashrate / 1e18), (int)(networkHashrate / 1e18) % 100);

        // Update current fee label
        lv_label_set_text_fmt(labels[2], "%lu sat/vB", currentFee);

        // update Network Difficulty label
        lv_label_set_text_fmt(labels[3], "%d.%02d T", (int)(networkDifficulty / 1e12), (int)(networkDifficulty / 1e12) % 100);

        // update BTC/EUR label
        lv_label_set_text_fmt(labels[4], "%d,%03d", (int)(btcPriceEUR/1000), (int)(btcPriceEUR) % 1000);

        // update Block Height label
        lv_label_set_text_fmt(labels[5], "%ld", blockHeight);

        // update Current Fee Low label
        lv_label_set_text_fmt(labels[6], "%lu sat/vB", currentFeeLow);

        // update Difficulty Progress Percent label
        lv_label_set_text_fmt(labels[7], "PROGRESS: %d.%02d%%", (int)(DifficultyProgressPercent), (int)(DifficultyProgressPercent * 100) % 100);

        // update Difficulty Change Percent label
        lv_label_set_text_fmt(labels[8], "CHANGE: %s%d.%02d%%", DifficultyChangePercent < 0 ? "-" : "", abs((int)DifficultyChangePercent), abs((int)(DifficultyChangePercent * 100) % 100));

        // difficulty progress bar
        lv_bar_set_value(labels[9], (int)(DifficultyProgressPercent), LV_ANIM_OFF);

        // btc CAD
        lv_label_set_text_fmt(labels[10], "%d,%03d", (int)(btcPriceCAD/1000), (int)(btcPriceCAD) % 1000);

        // halving countdown
        lv_label_set_text_fmt(labels[11], "%lu", halvingCountdown);

        // average fees
        lv_label_set_text_fmt(labels[12], "%lu sat/vB", averageFee);

        // difficulty remaining Progress Bar
        lv_bar_set_value(labels[13], (int)(DifficultyProgressPercent), LV_ANIM_OFF);

        // difficulty remaining Blocks
        lv_label_set_text_fmt(labels[14], "%ld BLOCKS", remainingBlocksToDifficultyAdjustment);
        
        // difficulty remaining time
        lv_label_set_text_fmt(labels[15], "%ld DAYS %ld HOURS",
            remainingTimeToDifficultyAdjustment / 86400,  // Convert seconds to days
            (remainingTimeToDifficultyAdjustment / 3600) % 24);  // Get hours (0-23)
        

        lvgl_port_unlock();
    }
}

void btcStatsScreen()
{
    uiTheme_t* theme = getCurrentTheme();

    static lv_obj_t* btcPriceContainer;
    static lv_obj_t* btcNetworkHashrateContainer;
    static lv_obj_t* btcCurrentFeeContainer;
    static lv_obj_t* btcNetworkDifficultyContainer;
    
    quadContainer(screenObjs.btcStatsMainContainer, btcPriceContainer, btcNetworkHashrateContainer, btcCurrentFeeContainer, btcNetworkDifficultyContainer);
 // add clickable flags
   lv_obj_add_flag(btcPriceContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(btcNetworkHashrateContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(btcCurrentFeeContainer, LV_OBJ_FLAG_CLICKABLE);
   lv_obj_add_flag(btcNetworkDifficultyContainer, LV_OBJ_FLAG_CLICKABLE);

    quadContainerContent(btcPriceContainer, "BTC/USD", "SYNC");
    quadContainerContent(btcNetworkHashrateContainer, "NETWORK HASHRATE", "SYNC");
    quadContainerContent(btcCurrentFeeContainer, "CURRENT FEES FAST", "SYNC");
    quadContainerContent(btcNetworkDifficultyContainer, "NETWORK DIFFICULTY", "SYNC");

    // Create Alternative containers hidden
    static lv_obj_t* btcPriceEURContainer;
    static lv_obj_t* btcBlockHeightContainer;
    static lv_obj_t* btcCurrentFeeLowContainer;
    static lv_obj_t* networkDiffPercentContainer;

    quadContainer(screenObjs.btcStatsMainContainer, btcPriceEURContainer, btcBlockHeightContainer, btcCurrentFeeLowContainer, networkDiffPercentContainer);
    // add clickable flags
    lv_obj_add_flag(btcPriceEURContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btcBlockHeightContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btcCurrentFeeLowContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(networkDiffPercentContainer, LV_OBJ_FLAG_CLICKABLE);

    quadContainerContent(btcPriceEURContainer, "BTC/EUR", "SYNC");
    quadContainerContent(btcBlockHeightContainer, "BLOCK HEIGHT", "SYNC");
    quadContainerContent(btcCurrentFeeLowContainer, "CURRENT FEES MIN", "SYNC");

    // create difficulty progresss bar for networkDiffPercentContainer
     static lv_style_t styleBarBG;
    static lv_style_t styleBarIndicator;
    lv_style_init(&styleBarIndicator);
    //lv_style_set_radius(&styleBar, LV_RADIUS_CIRCLE);
    lv_style_set_border_color(&styleBarIndicator, theme->borderColor);
    lv_style_set_bg_opa(&styleBarIndicator, LV_OPA_70);
    lv_style_set_bg_color(&styleBarIndicator, theme->primaryColor);
    lv_style_set_radius(&styleBarIndicator, 8);

    lv_style_init(&styleBarBG);
    lv_style_set_border_color(&styleBarBG, theme->borderColor);
    lv_style_set_bg_opa(&styleBarBG, LV_OPA_30);
    lv_style_set_bg_color(&styleBarBG, theme->primaryColor);
    lv_style_set_radius(&styleBarBG, 12);

    lv_obj_t* btcDifficultyProgressBar = lv_bar_create(networkDiffPercentContainer);
    lv_obj_set_size(btcDifficultyProgressBar, 295, 165);
    lv_obj_align(btcDifficultyProgressBar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(btcDifficultyProgressBar, &styleBarBG, LV_PART_MAIN);
    lv_obj_add_style(btcDifficultyProgressBar, &styleBarIndicator, LV_PART_INDICATOR);
    lv_bar_set_value(btcDifficultyProgressBar, (int)(IncomingData.api.difficultyProgressPercent), LV_ANIM_OFF);
    lv_obj_clear_flag(btcDifficultyProgressBar, LV_OBJ_FLAG_CLICKABLE);


    // create labels for the progress bar
    static lv_obj_t* btcDifficultyProgressLabel;
    static lv_obj_t* btcDifficultyChangeLabel;
    
    // Progress label
    btcDifficultyProgressLabel = lv_label_create(btcDifficultyProgressBar);
    lv_obj_align(btcDifficultyProgressLabel, LV_ALIGN_CENTER, 0, -16);
    lv_obj_set_style_text_font(btcDifficultyProgressLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcDifficultyProgressLabel, theme->textColor, LV_PART_MAIN);
    lv_label_set_text_fmt(btcDifficultyProgressLabel, "PROGRESS: %d.%02d%%", 
        (int)(IncomingData.api.difficultyProgressPercent), 
        (int)(IncomingData.api.difficultyProgressPercent * 100) % 100);
    lv_obj_set_style_text_align(btcDifficultyProgressLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(btcDifficultyProgressLabel, 295);

    // Change label
    btcDifficultyChangeLabel = lv_label_create(btcDifficultyProgressBar);
    lv_obj_set_style_text_font(btcDifficultyChangeLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcDifficultyChangeLabel, theme->textColor, LV_PART_MAIN);
    lv_obj_align(btcDifficultyChangeLabel, LV_ALIGN_CENTER, 0, 16);
    lv_label_set_text_fmt(btcDifficultyChangeLabel, "ESTIMATED CHANGE: %s%d.%02d%%",
        IncomingData.api.difficultyChangePercent < 0 ? "-" : "",
        abs((int)IncomingData.api.difficultyChangePercent),
        abs((int)(IncomingData.api.difficultyChangePercent * 100) % 100));
    lv_obj_set_style_text_align(btcDifficultyChangeLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(btcDifficultyChangeLabel, 295);


    // Create third container set
    static lv_obj_t* btcPriceCADContainer;
    static lv_obj_t* btcHalvingCountdownContainer;
    static lv_obj_t* btcAverageFeeContainer;
    static lv_obj_t* btcDifficultyRemainingTimeContainer;
    static lv_obj_t* btcDifficultyRemainingTimeProgressBar;
    static lv_obj_t* btcDifficultyRemainingBlocksLabel;
    static lv_obj_t* btcDifficultyRemainingTimeLabel;

    quadContainer(screenObjs.btcStatsMainContainer, btcPriceCADContainer, btcHalvingCountdownContainer, btcAverageFeeContainer, btcDifficultyRemainingTimeContainer);
    // add clickable flags
    lv_obj_add_flag(btcPriceCADContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btcHalvingCountdownContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btcAverageFeeContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btcDifficultyRemainingTimeContainer, LV_OBJ_FLAG_CLICKABLE);

    quadContainerContent(btcPriceCADContainer, "BTC/CAD", "SYNC");
    quadContainerContent(btcHalvingCountdownContainer, "HALVING COUNTDOWN", "SYNC");
    quadContainerContent(btcAverageFeeContainer, "AVERAGE FEES", "SYNC");
    
// create difficulty progresss bar for btcDifficultyRemainingTimeContainer
    static lv_style_t styleBarBG2;
    static lv_style_t styleBarIndicator2;
    lv_style_init(&styleBarIndicator2);
    //lv_style_set_radius(&styleBar, LV_RADIUS_CIRCLE);
    lv_style_set_border_color(&styleBarIndicator2, theme->borderColor);
    lv_style_set_bg_opa(&styleBarIndicator2, LV_OPA_70);
    lv_style_set_bg_color(&styleBarIndicator2, theme->primaryColor);
    lv_style_set_radius(&styleBarIndicator2, 8);

    lv_style_init(&styleBarBG2);
    lv_style_set_border_color(&styleBarBG2, theme->borderColor);
    lv_style_set_bg_opa(&styleBarBG2, LV_OPA_30);
    lv_style_set_bg_color(&styleBarBG2, theme->primaryColor);
    lv_style_set_radius(&styleBarBG2, 12);

    btcDifficultyRemainingTimeProgressBar = lv_bar_create(btcDifficultyRemainingTimeContainer);
    lv_obj_set_size(btcDifficultyRemainingTimeProgressBar, 295, 165);
    lv_obj_align(btcDifficultyRemainingTimeProgressBar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(btcDifficultyRemainingTimeProgressBar, &styleBarBG2, LV_PART_MAIN);
    lv_obj_add_style(btcDifficultyRemainingTimeProgressBar, &styleBarIndicator2, LV_PART_INDICATOR);
    lv_bar_set_value(btcDifficultyRemainingTimeProgressBar, (int)(IncomingData.api.remainingBlocksToDifficultyAdjustment), LV_ANIM_OFF);
    lv_obj_clear_flag(btcDifficultyRemainingTimeProgressBar, LV_OBJ_FLAG_CLICKABLE);
    
    // Progress label
    btcDifficultyRemainingBlocksLabel = lv_label_create(btcDifficultyRemainingTimeContainer);
    lv_obj_align(btcDifficultyRemainingBlocksLabel, LV_ALIGN_CENTER, 0, -16);
    lv_label_set_text_fmt(btcDifficultyRemainingBlocksLabel, "REMAINING BLOCKS: %d", 
        (int)(IncomingData.api.remainingBlocksToDifficultyAdjustment));
    lv_obj_set_style_text_font(btcDifficultyRemainingBlocksLabel, theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcDifficultyRemainingBlocksLabel, theme->textColor, LV_PART_MAIN);

    // Change label
    btcDifficultyRemainingTimeLabel = lv_label_create(btcDifficultyRemainingTimeContainer);
    lv_obj_align(btcDifficultyRemainingTimeLabel, LV_ALIGN_CENTER, 0, 16);
    lv_label_set_text_fmt(btcDifficultyRemainingTimeLabel, "REMAINING TIME: %ld days %ld hours",
        IncomingData.api.remainingTimeToDifficultyAdjustment / 86400,  // Convert seconds to days
        (IncomingData.api.remainingTimeToDifficultyAdjustment / 3600) % 24);  // Get hours (0-23)
    lv_obj_set_style_text_font(btcDifficultyRemainingTimeLabel,  theme->fontMedium24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btcDifficultyRemainingTimeLabel, theme->textColor, LV_PART_MAIN);
    // Get the value labels created by quadContainerContent
    static lv_obj_t* btcStatsLabels[16] = {
        lv_obj_get_child(btcPriceContainer, 1),
        lv_obj_get_child(btcNetworkHashrateContainer, 1),
        lv_obj_get_child(btcCurrentFeeContainer, 1),
        lv_obj_get_child(btcNetworkDifficultyContainer, 1),
        lv_obj_get_child(btcPriceEURContainer, 1),
        lv_obj_get_child(btcBlockHeightContainer, 1),
        lv_obj_get_child(btcCurrentFeeLowContainer, 1),
        btcDifficultyProgressLabel,
        btcDifficultyChangeLabel,
        btcDifficultyProgressBar,
        lv_obj_get_child(btcPriceCADContainer, 1),
        lv_obj_get_child(btcHalvingCountdownContainer, 1),
        lv_obj_get_child(btcAverageFeeContainer, 1),
        btcDifficultyRemainingTimeProgressBar,
        btcDifficultyRemainingBlocksLabel,
        btcDifficultyRemainingTimeLabel
    };

    // Create timer to update labels every .5 seconds
    screenObjs.btcStatsLabelsUpdateTimer = lv_timer_create(updateBtcStatsLabels, 500, btcStatsLabels);

    // After creating first set of containers
    containerStates[0].containers[0] = btcPriceContainer;
    containerStates[0].containers[1] = btcPriceEURContainer;
    containerStates[0].containers[2] = btcPriceCADContainer;
    containerStates[0].currentIndex = 0;

    // After creating second set of containers
    containerStates[1].containers[0] = btcNetworkHashrateContainer;
    containerStates[1].containers[1] = btcBlockHeightContainer;
    containerStates[1].containers[2] = btcHalvingCountdownContainer;
    containerStates[1].currentIndex = 0;

    // After creating third set of containers
    containerStates[2].containers[0] = btcCurrentFeeContainer;
    containerStates[2].containers[1] = btcCurrentFeeLowContainer;
    containerStates[2].containers[2] = btcAverageFeeContainer;
    containerStates[2].currentIndex = 0;

    // After creating fourth set of containers
    containerStates[3].containers[0] = btcNetworkDifficultyContainer;
    containerStates[3].containers[1] = networkDiffPercentContainer;
    containerStates[3].containers[2] = btcDifficultyRemainingTimeContainer;
    containerStates[3].currentIndex = 0;

    // Add event callback to all containers
    for (int group = 0; group < 4; group++) {
        for (int i = 0; i < btcStats_MAX_VARIATIONS; i++) {
            lv_obj_add_event_cb(containerStates[group].containers[i], quadContainerEventCallback, LV_EVENT_CLICKED, NULL);
        }
    }

    // Initialize visibility
    for (int group = 0; group < 4; group++) {
        updateContainerVisibility(group);
    }
}

