| Supported ESP SoCs | ESP32 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ------------------ | ----- | -------- | -------- | -------- | -------- | -------- |

| Supported LCD Controllers | GC9A01 | GC9B71 | ILI9341 | NV3022B | SH8601 | SPD2010 | ST7789 | ST77916 | ST77922 |
| ------------------------- | ------ | ------ | ------- | ------- | ------ | ------- | ------ | ------- | ------- |

# SPI LCD Example

The example demonstrates how to develop different model LCDs with SPI interface using standalone drivers and test them by displaying color bars.

## How to use

1. [Configure drivers](../../../docs/How_To_Use.md#configuring-drivers) if needed.
2. Modify the macros in the example to match the parameters according to your hardware.
3. Navigate to the `Tools` menu in the Arduino IDE to choose a ESP board and configure its parameters, please refter to [Configuring Supported Development Boards](../../../docs/How_To_Use.md#configuring-supported-development-boards)
4. Verify and upload the example to your ESP board.

## Serial Output

```
...
SPI LCD example start
Initialize backlight control pin and turn it off
Create SPI LCD bus
Create LCD device
Draw color bar from top left to bottom right, the order is B - G - R
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Draw bitmap finish callback
Turn on the backlight
SPI LCD example end
IDLE loop
...
```

## Troubleshooting

Please check the [FAQ](../../../docs/FAQ.md) first to see if the same question exists. If not, please create a [Github issue](https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues). We will get back to you as soon as possible.
