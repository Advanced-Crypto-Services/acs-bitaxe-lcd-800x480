#include "UIThemes.h"
#include "fonts.h"
#include "modelConfig.h"
#include <SPIFFS.h>
#include "BAP.h"

uiTheme_t currentTheme;

// Default theme values (Original ACS green theme)
static const uiTheme_t defaultTheme = {
    .primaryColor = lv_color_hex(0xA7F3D0),
    .secondaryColor = lv_color_hex(0xA7F3D0),
    .backgroundColor = lv_color_hex(0x161f1b),
    .textColor = lv_color_hex(0xA7F3D0),
    .borderColor = lv_color_hex(0xA7F3D0),
    .defaultOpacity = 80,
    .backgroundOpacity = 40,
    .fontExtraBold144 = &interExtraBold144,
    .fontExtraBold72 = &interExtraBold72,
    .fontExtraBold56 = &interExtraBold56,
    .fontExtraBold32 = &interExtraBold32,
    .fontMedium24 = &interMedium24,
    .fontMedium16 = &interMedium16_19px,
    .background = "S:/UIBackgroundACSDEFAULT.png",
    .logo1 = "S:/Logos.png",
    .logo2 = "S:/openSourceBitcoinMining.png",
    .themePreview = "S:/acsGreenUIPreview.png",
    .themePreset = THEME_ACS_DEFAULT,
    .themeName = "THEME_ACS_DEFAULT"
};

uiTheme_t* getCurrentTheme(void) {
    return &currentTheme;
}

themePreset_t getCurrentThemePreset(void) {
    return currentTheme.themePreset;
}

bool isThemeAvailable(themePreset_t theme) {
    switch (theme) {
        case THEME_ACS_DEFAULT:
            return true; // Default theme is always available
        case THEME_BITAXE_RED:
            return true;
        case THEME_BLOCKSTREAM_JADE:
            return SPIFFS.exists("/UILogoJADE.png");
        case THEME_BLOCKSTREAM_BLUE:
            return SPIFFS.exists("/bsBlueLogo.png");
        case THEME_SOLO_SATOSHI:
            return SPIFFS.exists("/SoloSatoshiSmallLogo.png");
        case THEME_SOLO_MINING_CO:
            return SPIFFS.exists("/soloMiningCo.png");
        default:
            return false;
    }
}

void initializeTheme(themePreset_t preset) {
    switch (preset) {
        
        case THEME_BITAXE_RED:
                currentTheme = (uiTheme_t){
                    .primaryColor = lv_color_hex(0xF80421),
                    .secondaryColor = lv_color_hex(0xfc4d62),
                    .backgroundColor = lv_color_hex(0x070D17),
                    .textColor = lv_color_hex(0xF80421),
                    .borderColor = lv_color_hex(0xfc4d62),
                    .defaultOpacity = 80,
                    .backgroundOpacity = 40,
                    .fontExtraBold144 = &interExtraBold144,
                    .fontExtraBold72 = &interExtraBold72,
                    .fontExtraBold56 = &interExtraBold56,
                    .fontExtraBold32 = &interExtraBold32,
                    .fontMedium24 = &interMedium24,
                    .fontMedium16 = &interMedium16_19px,
                    .background = "S:/UIBackgroundBITAXERED.png",
                    .logo1 = "S:/Logos.png",
                    .logo2 = "S:/openSourceBitcoinMining.png",
                    .themePreview = "S:/bitaxeRedUIPreview.png",
                    .themePreset = THEME_BITAXE_RED,
                    .themeName = "THEME_BITAXE_RED"
                };
            break;
            
        case THEME_BLOCKSTREAM_JADE:
            if (SPIFFS.exists("/UILogoJADE.png")) {
                currentTheme = (uiTheme_t){
                    .primaryColor = lv_color_hex(0x00B093),
                    .secondaryColor = lv_color_hex(0x006D62),
                    .backgroundColor = lv_color_hex(0x111316),
                    .textColor = lv_color_hex(0x21CCAB),
                    .borderColor = lv_color_hex(0x01544A),
                    .defaultOpacity = 80,
                    .backgroundOpacity = 40,
                    .fontExtraBold144 = &interExtraBold144,
                    .fontExtraBold72 = &interExtraBold72,
                    .fontExtraBold56 = &interExtraBold56,
                    .fontExtraBold32 = &interExtraBold32,
                    .fontMedium24 = &interMedium24,
                    .fontMedium16 = &interMedium16_19px,
                    .background = "S:/UIBackgroundJADEGREEN.png",
                    .logo1 = "S:/Logos.png",
                    .logo2 = "S:/UILogoJADE.png",
                    .themePreview = "S:/jadeUIPreview.png",
                    .themePreset = THEME_BLOCKSTREAM_JADE,
                    .themeName = "THEME_BLOCKSTREAM_JADE"
                };
            } else {
                currentTheme = defaultTheme;
            }
            break;

        case THEME_BLOCKSTREAM_BLUE:
            if (SPIFFS.exists("/bsBlueLogo.png")) {
                currentTheme = (uiTheme_t){
                    .primaryColor = lv_color_hex(0x00C3FF),
                    .secondaryColor = lv_color_hex(0x00C3FF),
                    .backgroundColor = lv_color_hex(0x111316),
                    .textColor = lv_color_hex(0x00C3FF),
                    .borderColor = lv_color_hex(0x00C3FF),
                    .defaultOpacity = 80,
                    .backgroundOpacity = 40,
                    .fontExtraBold144 = &interExtraBold144,
                    .fontExtraBold72 = &interExtraBold72,
                    .fontExtraBold56 = &interExtraBold56,
                    .fontExtraBold32 = &interExtraBold32,
                    .fontMedium24 = &interMedium24,
                    .fontMedium16 = &interMedium16_19px,
                    .background = "S:/UIBackgroundBLKSTRMBLUE.png",
                    .logo1 = "S:/Logos.png",
                    .logo2 = "S:/bsBlueLogo.png",
                    .themePreview = "S:/bsBlueUIPreview.png",
                    .themePreset = THEME_BLOCKSTREAM_BLUE,
                    .themeName = "THEME_BLOCKSTREAM_BLUE"
                };
            } else {
                currentTheme = defaultTheme;
            }
            break;
            
        case THEME_SOLO_SATOSHI:
            if (SPIFFS.exists("/SoloSatoshiSmallLogo.png")) {
                currentTheme = (uiTheme_t){
                    .primaryColor = lv_color_hex(0xF80421),
                    .secondaryColor = lv_color_hex(0xf7931a),
                    .backgroundColor = lv_color_hex(0x070D17),
                    .textColor = lv_color_hex(0xFFFFFF),
                    .borderColor = lv_color_hex(0xf7931a),
                    .defaultOpacity = 80,
                    .backgroundOpacity = 40,
                    .fontExtraBold144 = &interExtraBold144,
                    .fontExtraBold72 = &interExtraBold72,
                    .fontExtraBold56 = &interExtraBold56,
                    .fontExtraBold32 = &interExtraBold32,
                    .fontMedium24 = &interMedium24,
                    .fontMedium16 = &interMedium16_19px,
                    .background = "S:/UIBackgroundBITAXERED.png",
                    .logo1 = "S:/Logos.png",
                    .logo2 = "S:/SoloSatoshiSmallLogo.png",
                    .themePreview = "S:/SoloSatoshiSmallLogo.png",
                    .themePreset = THEME_SOLO_SATOSHI,
                    .themeName = "THEME_SOLO_SATOSHI"
                };
            } else {
                currentTheme = defaultTheme;
            }
            break;
 
            
        case THEME_SOLO_MINING_CO:
            if (SPIFFS.exists("/soloMiningCo.png")) {
                currentTheme = (uiTheme_t){
                    .primaryColor = lv_color_hex(0xf15900),
                    .secondaryColor = lv_color_hex(0xc5900F1),
                    .backgroundColor = lv_color_hex(0x111316),
                    .textColor = lv_color_hex(0xffffffff),
                    .borderColor = lv_color_hex(0xc5900F1),
                    .defaultOpacity = 80,
                    .backgroundOpacity = 40,
                    .fontExtraBold144 = &interExtraBold144,
                    .fontExtraBold72 = &interExtraBold72,
                    .fontExtraBold56 = &interExtraBold56,
                    .fontExtraBold32 = &interExtraBold32,
                    .fontMedium24 = &interMedium24,
                    .fontMedium16 = &interMedium16_19px,
                    .background = "S:/UIBackgroundBITAXERED.png",
                    .logo1 = "S:/Logos.png",
                    .logo2 = "S:/soloMiningCo.png",
                    .themePreview = "S:/soloMiningCo.png",
                    .themePreset = THEME_SOLO_MINING_CO,
                    .themeName = "THEME_SOLO_MINING_CO"
                };
            } else {
                currentTheme = defaultTheme;
            }
            break;
        
        default:
            currentTheme = defaultTheme;
            break;
    }
    
    // Write theme information to BAP
    char themeInfo[MAX_THEME_LENGTH];
    snprintf(themeInfo, sizeof(themeInfo), "THEME:%s", currentTheme.themeName);
    writeDataToBAP((uint8_t*)currentTheme.themeName, strlen(currentTheme.themeName), LVGL_REG_THEME_CURRENT);
}