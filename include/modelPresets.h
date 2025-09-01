#pragma once
#include "Arduino.h"
#include "BAP.h"
#include "modelConfig.h"


extern uint16_t currentPresetFrequency;
extern uint16_t currentPresetVoltage;
extern uint8_t currentPresetFanSpeed;
extern bool currentPresetAutoFanMode;

extern const uint16_t voltageQuiet;
extern const uint16_t voltageBalanced;
extern const uint16_t voltageTurbo;

extern const uint16_t freqQuiet;
extern const uint16_t freqBalanced;
extern const uint16_t freqTurbo;

extern uint16_t frequencyOffset;
extern uint16_t voltageOffset;
extern uint16_t fanSpeedOffset; 

extern bool autoTuneEnabled;
extern uint16_t autotuneTempLowTarget;
extern uint16_t autotuneTempHighTarget;



extern void setQuietPreset();
extern void setBalancedPreset();
extern void setTurboPreset();


