# Soil Monitor
My wife waqs given a succulent as a gift, but she was worried about overwatering it - a commn way to kill a succulent. So, to help, I decided to build a 
soil monitor and configure it specifically for succulents.

The result is a battery-powered soil moisture monitor with a clear OLED display and ultra-low power sleep mode. Designed to be compact and dead-simple for 
plant lovers who just want to know when to water.

# Project Goal
A compact, effective monitor for soil moisture with
  A simple, low power display that shows
      Moisture percentage
      What to do with that info (Showing 'OK' or 'Needs water' depending on moisture) 
      A battery icon that conveys visually how much power remains
      A percentage display cpnveying how much power remains
  A 3.7 V LiPo (charged via USB-C) for wireless power
  A sleep mode that saves battery after inactivity and powers down much of the internals
  A wake button to instantly get the onitor ready for work
  Accurate soil readings via capacitive sensor
  A tiny SoC to run the show (the Seeed Studio XIAO nRF52840 which is ~18mm X 21mm)
  Recharge capability (built into the nRF52840)

# Wiring

| Component                  | XIAO Pin          |
|---------------------------|-------------------|
| OLED (SSD1306, I2C)       | SDA → GPIO 4<br>SCL → GPIO 5 |
| Moisture Sensor           | A1 (Pin 1)        |
| Battery Voltage Divider   | A3 (Pin 3) via 100kΩ–100kΩ |
| Wake Button               | GPIO 2 → GND      |
| Battery                   | BAT+ pad + GND pin|

![Wiring Diagram](https://github.com/ChandlerEx/Projects/blob/891a760c9f960f25a479301b3d8ec1b8d20b8800/SoilMonitor/SoilMonWiring.png)

# Calibration
Moisture readings are calibrated between:
  *Dry:* `2665`
  *Wet:* `1155`
Mapped to 0–100% for display.

# Enclosure
  Designed in Fusion 360 (
- Custom 3D-printed shell with a translucent PETG window
- Bottom plate removable to access LiPo battery
- 

# Hardware
ESP32-3248S035R (3.5" 320x480 TFT + resistive touch)
Backlight dimming via GPIO21
USB-powered; internal JST used for streamlined cable exit
Enclosure custom designed from scratch and built with AM and Fusion 360

# Architecture Overview
Coded in Arduino-style C++
Touch panel: Short tap toggles display mode (score ↔ clock), long-press enters settings
Display modes:
    MODE_SCOREBOARD – live game status, scores, inning, runners on base
    MODE_CLOCK – digital clock + date
    MODE_SETTINGS – flip screen, toggle clock, reset Wi-Fi
Data source: Live API from statsapi.mlb.com
Auto-reset: Refreshes at 3AM PT or after long uptime; boots in seconds
Wi-Fi setup: Built-in captive portal (WiFiManager) on first boot or reset

# Photos
Early Proto
![EarlyProto](https://github.com/user-attachments/assets/f6440fe7-e67e-47fb-aa2f-fecacd956c5a)

Current Build
![CurrentScoreboard](https://github.com/user-attachments/assets/ccfb72b6-7670-484e-8638-d6ba8d4bcea1)

# Lessons Learned
Time zones suck. Using UTC parsing + Pacific re-shifting was necessary.
Some Yankees games end with no bottom of 9th — needed a failsafe for final detection.
Display spacing and font choices were tuned for clarity on 240x320 screen.
Adding the JST wires inside for power made the enclosure much, much cleaner than using USB
Coding for the ESP32 is extremely touchy, especialy with fonts (FreeFont vs TextFont)
Making enclosure adjustments when the front plane is at a 60 deg angle is brutal

# Files
MLB StatsAPI: https://statsapi.mlb.com
Board Specs: Stored in Repo
Full ESP32 sketch: ScoreboardPrototype.ino
Full Fusion Archive: Scoreboard.f3d
STL files of enclosure and bezel: Scoreboard.stl

# Potential Future Features
Allow users to adjust brightness of the various displays in the settings and only with long or short screen taps
Allow users to follow any MLB team they want (configuring that in the CP when getting initially set up
Add other sports as well (NFL, NBA, MLS, NHL, F1)
Add a sneaky, unobtrusive icon to the clock screen to indicate a game is currently being played# Soil Moisture Monitor 🌱




*******************************************************************

A battery-powered soil moisture monitor with a clear OLED display and ultra-low power sleep mode. Designed to be compact, cute, and dead-simple for plant lovers who just want to know when to water.

![Soil Monitor in Enclosure](images/soil_monitor_photo.jpg)


See full wiring diagram in `/docs/wiring.png`



## Portfolio Intent

This is one of three key projects in my electronics portfolio, alongside the [Scoreboard](https://github.com/yourname/scoreboard) and [StockPriceOrb](https://github.com/yourname/stockorb). It showcases:

- Sensor calibration & analog input
- Power management
- Embedded UI design
- PCB-free low-power prototyping

## Media

Photos and build video in `/media/`
