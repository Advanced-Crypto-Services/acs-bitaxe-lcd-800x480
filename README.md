# 800x480 Screen Add-On for ESP-Miner for Bitaxe Devices

# Features
The Display has a UI built around making the Bitaxe easier to setup for new users, as well as displaying useful data about both the bitaxe as well as the bitcoin network.
## Clock App
![clock](https://github.com/user-attachments/assets/6460f6f8-b876-4093-869d-65e22d126602)
## BTC Network Page
![btc](https://github.com/user-attachments/assets/c09b1477-a1ac-4a95-8ff1-c1095743bca2)
## Live Hashrate Graph
![graph](https://github.com/user-attachments/assets/883fb0fa-9656-4e1b-bc1e-83c6c7724c15)
## Activity Screen
![activity](https://github.com/user-attachments/assets/489f8fcc-ebb8-440c-8097-288d82179537)
## Multiple Themes
![themes](https://github.com/user-attachments/assets/72587bb3-01df-4dfb-8196-f8cf96fe109f)
## Setting Screens
![settings](https://github.com/user-attachments/assets/26890be8-e027-469e-b222-749325916e6a)

## Initial Setup Made Easy
![initial setup](https://github.com/user-attachments/assets/c2dd6f02-b743-44c3-b04e-cb0f09b9aa46)

## Based On [Wavshare ESP32S3 4.3" LCD](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3)
![image](https://github.com/user-attachments/assets/b7c3f45b-9023-491d-a716-e8a729b3063f)


## Hardware Changes neccessary 
The Waveshare display does not have a serial connection, and non of the other connections would not suffice for communication.
![Modified Example](https://github.com/user-attachments/assets/1794412b-16e4-4e2f-8d72-cd320f954c70)


## Data Interface
We modfify the I2C connector to connect to the ESP on the display via UART.
Currently, We utilize Pin GPIO15 and 16 for this new Bus, We can do this by removing 3 resistors and 2 ICs for the I2c and RS-485>UART circuits
A custom Register based scheme for keeping track of what data is being brodcasted.

See more in BAP.cpp 

The current layout looks like this:
- Byte 1: Register number
- Byte 2: Data length
- Byte 3+: Data

## Register Numbers
### Network Registers
- SSID        0x21
- IP Address      0x22
- Wifi Status    0x23
- Pool URL       0x24
- Pool 2 URL   0x25
- Pool Port Numbers (1 and 2)     0x26

### Mining Data Registers
- Current Hashrate       0x30
- Historical Hashrate   0x31
- Current Efficiency      0x32
- Best Difficulty All Time       0x33
- Best Difficulty Session    0x34
- Accepted Shares, Rejected Shares          0x35

### Monitoring Data Regesters
- Asic Temp          0x40
- Asic Frequnecy      0x41
- Fan Percentage and Speed            0x42
- VIN, CurrentIN, Wattage Used, Domain Voltage     0x43
- Asic Ingo     0x44
- Uptime         0x45 

### Device Status Registers
- Flags (Block Found, Startup Done, Overheat Mode)          0x50
- Device Info   0x52
- Board Info    0x53
- Clock Ntime Sync    0x54

### Mempool API Registers
- BTC Price   0x60
- Network Hashrate 0x61
- Network Difficulty 0x62
- Current Block Height 0x63
- Percentage Through Current Difficuty Adjustment Period  0x64
- Estimate Difficukty Change Percentage 0x65
- Remaing Block in Current Difficulty Period 0x66
- Remaining Time in Current Difficulty Period 0x67

### Settings Registers
Network Hostname 0x90
NEtwork SSID 0x91
Network Password 0x92

Pool Settings 0x93 - 0x9A
Asic Voltage 0x9B
Asic Frequency 0x9C
Fan Speed 0x9D
Auto Fan Speed 0x9E


### Special Registers
Restart 0xFE
Startup Done 0xE0
Overheat Mode 0xE1
Found Block 0xE2

## Requirements

### VSCode
### [LVGLv8.3](https://lvgl.io/)
### [ESP32_Display_Panel](https://github.com/espressif/esp-display-panel)
### [ESP32_IO_Expander](https://github.com/espressif/esp-io-expander)
### [ESP32 Arduino](https://github.com/espressif/arduino-esp32)
### [TimeLib](https://github.com/PaulStoffregen/Time)

## Library Configuration
Libraries can be found in the managed compoenets firectory


# Related Projects:
### [ACS Bitaxe Hardware](https://github.com/Advanced-Crypto-Services/acs-esp-miner)
### [800x480 LCD Add-On](https://github.com/Advanced-Crypto-Services/acs-bitaxe-lcd-800x480)
