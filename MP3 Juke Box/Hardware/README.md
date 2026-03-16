# Hardware Design – ESP32 MP3 Jukebox

This folder contains the complete KiCad project files for the custom
ESP32-based MP3 Jukebox PCB.

## Overview

The PCB integrates:

- ESP32-WROOM-32 Dev Board
- DFPlayer Mini (DFR0299)
- SSD1306 128x64 I2C OLED
- PAM8403 Audio Amplifier
- Pushbutton GPIO inputs
- Analog volume control (ADC)
- 5V USB-C power input

The design is a 2-layer PCB with dedicated power and signal routing.

---

## Files Included

- `.kicad_pro` – KiCad project file  
- `.kicad_sch` – Complete schematic  
- `.kicad_pcb` – PCB layout  
- `Gerbers/` – Manufacturing-ready Gerber files  
- `Libraries/` – External symbol and footprint files used  

---

## Manufacturing

The Gerber files were generated using KiCad 9.0.

To fabricate:

1. Upload the `Gerbers/` folder as a zip file to:
   - JLCPCB
   - PCBWay
   - Any standard PCB manufacturer

2. Standard settings:
   - 2-layer board
   - 1.6mm thickness
   - HASL finish
   - 1oz copper

---

## Design Notes

- Separate analog and digital routing where possible.
- Audio lines kept short to minimize noise.
- Power rails routed with wider traces.
- Mounting holes included for enclosure integration.

---

## External Libraries Used

- DFPlayer Mini (DFR0299) – SnapEDA  
- ESP32 DevKit – Espressif KiCad Libraries  
- SSD1306 OLED – KiCad-SSD1306-128x64  
- PAM8403 Amplifier – pam8403-module-kicad-lib  

See main README for full repository links.