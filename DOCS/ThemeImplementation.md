# Theme Implementation Documentation

## Overview
The theme system in the BitaxeTouch LCD Firmware is designed to provide a flexible and dynamic way to customize the user interface. The implementation consists of several key components that work together to manage themes, their availability, and their application to the UI.

## Core Components

### 1. Theme Definition (`UIThemes.h`)
The theme system is built around two main data structures:

#### Theme Preset Enum
```cpp
typedef enum {
    THEME_ACS_DEFAULT = 0, 
    THEME_BITAXE_RED = 1,
    THEME_BLOCKSTREAM_JADE = 2,
    THEME_BLOCKSTREAM_BLUE = 3,
    THEME_SOLO_SATOSHI = 4,
    THEME_SOLO_MINING_CO = 5
} themePreset_t;
```

#### Theme Structure
```cpp
typedef struct {
    lv_color_t primaryColor;      // Main brand color
    lv_color_t secondaryColor;    // Secondary brand color
    lv_color_t backgroundColor;   // Background color
    lv_color_t textColor;         // Text color
    lv_color_t borderColor;       // Border color
    uint8_t defaultOpacity;       // Default opacity for elements
    uint8_t backgroundOpacity;    // Background opacity
    lv_font_t* fontExtraBold144;  // Font definitions
    lv_font_t* fontExtraBold72;
    lv_font_t* fontExtraBold56;
    lv_font_t* fontExtraBold32;
    lv_font_t* fontMedium24;
    lv_font_t* fontMedium16;
    const char* background;       // Background image path
    const char* logo1;            // Primary logo path
    const char* logo2;            // Secondary logo path
    const char* themePreview;     // Theme preview image path
    themePreset_t themePreset;    // Theme identifier
} uiTheme_t;
```

### 2. Theme Management (`UIThemes.cpp`)

#### Theme Availability Check
```cpp
bool isThemeAvailable(themePreset_t theme) {
    switch (theme) {
        case THEME_ACS_DEFAULT:
            return true; // Default theme is always available
        case THEME_BITAXE_RED:
            return SPIFFS.exists("/bitaxe_red.marker");
        // ... other theme checks ...
    }
}
```
This function checks for the existence of theme marker files in the SPIFFS filesystem to determine if a theme is available.

#### Theme Initialization
```cpp
void initializeTheme(themePreset_t preset) {
    switch (preset) {
        case THEME_BITAXE_RED:
            if (SPIFFS.exists("/bitaxe_red.marker")) {
                // Set theme properties
            } else {
                currentTheme = defaultTheme;
            }
            break;
        // ... other theme cases ...
    }
}
```
This function initializes a theme based on its preset, falling back to the default theme if the requested theme is not available.

### 3. UI Integration (`UIScreens.cpp`)

#### Theme Dropdown
The theme selection is implemented through a dropdown menu that:
1. Only shows available themes (those with marker files)
2. Updates the UI when a theme is selected
3. Falls back to the default theme if an unavailable theme is selected

```cpp
// Theme dropdown creation and population
themeDropdown = lv_dropdown_create(themeSettingsContainer);
lv_dropdown_set_options(themeDropdown, "ACS DEFAULT\nBITAXE RED");

// Add available themes
if (isThemeAvailable(THEME_BLOCKSTREAM_JADE)) {
    lv_dropdown_add_option(themeDropdown, "BLOCKSTREAM JADE", LV_DROPDOWN_POS_LAST);
}
// ... other theme options ...
```

#### Theme Event Handler
```cpp
static void themeDropdownEventHandler(lv_event_t* e) {
    // Handle theme selection
    // Update UI elements with new theme
    // Fall back to default if theme is unavailable
}
```

## Theme Files Structure

### SPIFFS Files
Themes are managed through marker files in the SPIFFS filesystem:
- `/bitaxe_red.marker`
- `/jade.marker`
- `/blue.marker`
- `/solo_satoshi.marker`
- `/altair.marker`
- `/solo_mining.marker`
- `/btc_magazine.marker`
- `/voskcoin.marker`
- `/american_btc.marker`
- `/hut8.marker`
- `/luxor.marker`

### Theme Assets
Each theme includes:
1. Background images
2. Logo files
3. Theme preview images
4. Marker file for availability check

## Theme Application Process

1. **Theme Selection**
   - User selects a theme from the dropdown
   - System checks if theme is available
   - If available, theme is applied
   - If unavailable, falls back to default theme

2. **Theme Application**
   - Colors are applied to UI elements
   - Fonts are updated
   - Images are loaded
   - UI is refreshed

3. **Fallback Mechanism**
   - Default theme is always available
   - Unavailable themes are not shown in dropdown
   - Invalid selections revert to default theme

## Best Practices

1. **Theme Development**
   - Always include a marker file for new themes
   - Provide all required assets
   - Test theme availability checks
   - Ensure fallback to default theme works

2. **Theme Management**
   - Keep theme assets organized in SPIFFS
   - Use consistent naming conventions
   - Document theme requirements
   - Test theme switching thoroughly

3. **Performance Considerations**
   - Minimize theme asset sizes
   - Cache frequently used assets
   - Optimize theme switching process
   - Handle memory efficiently

## Future Improvements

1. **Dynamic Theme Loading**
   - Load themes from external storage
   - Support theme updates
   - Add theme validation

2. **Theme Customization**
   - Allow user-defined themes
   - Support theme mixing
   - Add theme export/import

3. **Performance Optimization**
   - Implement theme caching
   - Optimize asset loading
   - Reduce memory usage 