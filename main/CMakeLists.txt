idf_component_register(
    SRCS 
    "main.cpp" 
    "lvgl_port_v8.cpp"
    "BAP.cpp"
   
    "initialSetupScreen.cpp"
    "interExtraBold32.c"
    "interExtraBold56.c"
    "interExtraBold72.c"
    "interExtraBold144.c"
    "interMedium16_19px (1).c"
    "interMedium24.c"
    "mempoolAPI.c"
    "NVS.cpp"
    "spiffsDriver.cpp"
    "UIThemes.cpp"
    "UIHandler.cpp"
    "UIScreens.cpp"
    "UISharedAssets.cpp"
    "wifiFeatures.cpp"
    "Time.cpp"
    "DateStrings.cpp"
    "httpServer.cpp"
    "Clock.cpp"
    "modelPresets.cpp"
    "lcdSettings.cpp"
    "homeScreen.cpp"
    "activityScreen.cpp"
    "btcStats.cpp"
    "miningStatsScreen.cpp"
    "tutorialScreen.cpp"
    INCLUDE_DIRS 
    "."
    "../include"

)
spiffs_create_partition_image(spiffs ../data FLASH_IN_PROJECT)