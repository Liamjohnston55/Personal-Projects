# Firmware Information

Platform: ESP32-WROOM-32  
IDE: Arduino IDE 2.3.8  

## Pin Assignments

| Function      | GPIO |
|--------------|------|
| BTN_UP       | 13   |
| BTN_DOWN     | 27   |
| BTN_PLAY     | 14   |
| BTN_BACK     | 12   |
| Volume (ADC) | 34   |
| DFPlayer RX  | 16   |
| DFPlayer TX  | 17   |
| OLED SDA     | 21   |
| OLED SCL     | 22   |

## Required Libraries

- Adafruit SSD1306
- Adafruit GFX
- DFRobotDFPlayerMini

## Upload Instructions

1. Install ESP32 board support.
2. Install required libraries.
3. Select "ESP32 Dev Module".
4. Upload `mp3_jukebox.ino`.