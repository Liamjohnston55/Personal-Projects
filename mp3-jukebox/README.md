# ESP32 MP3 Jukebox

Custom embedded audio system designed using an ESP32 microcontroller.  
This project includes firmware development, custom PCB design in KiCad, and a 3D printed enclosure.

The system reads MP3 files from a microSD card and provides button-based navigation with OLED display feedback.

---

## System Overview

The jukebox consists of:

- ESP32-WROOM-32 (main controller)
- DFPlayer Mini (DFR0299) MP3 module
- PAM8403 3W Class-D audio amplifier
- 0.96" SSD1306 I2C OLED display
- Pushbutton user interface
- Analog volume control
- 18650 battery-powered system

The ESP32 handles user input, display rendering, power management, and DFPlayer communication over UART.

---

## Features

- Menu-based song selection
- OLED track display
- Loop playback
- Analog volume control
- Deep sleep power management
- Custom PCB layout
- Custom 3D printed enclosure

---

## Hardware

All hardware was designed and integrated manually.

PCB Design:
- Designed in KiCad
- Two-layer board
- Custom routing and layout
- Gerber files included for manufacturing

Location:
`Hardware/ESP32_MP3_Jukebox/`

---

## Firmware

- Arduino framework for ESP32
- UART2 communication with DFPlayer
- I2C OLED display control
- Button debouncing logic
- Deep sleep implementation

Location:
`Code/`

---

## Mechanical Design

The enclosure and mounting components were designed for 3D printing.

Location:
`Mechanical/`

---

## Manufacturing

Gerber files are included for PCB fabrication.

`Hardware/ESP32_MP3_Jukebox/ESP32_MP3_Jukebox_Gerbers/`

---

## External Libraries and References

The following third-party resources were used for schematic symbols and PCB footprints:

- **DFPlayer Mini (DFR0299)** – SnapEDA  
  https://www.snapeda.com/parts/DFR0299/DFRobot/view-part/

- **ESP32 KiCad Libraries** – Espressif  
  https://github.com/espressif/kicad-libraries

- **SSD1306 128x64 OLED KiCad Library** – pforrmi  
  https://github.com/pforrmi/KiCad-SSD1306-128x64

- **PAM8403 Module KiCad Library** – johnnycubides  
  https://github.com/johnnycubides/pam8403-module-kicad-lib

All third-party libraries remain property of their respective authors and are used in accordance with their licenses.