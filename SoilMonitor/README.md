# Soil Monitor
My wife got a succulent as a gift, but she was worried about overwatering it - a common way to kill a succulent! So, to help, I decided to build a 
soil monitor and configure it specifically for succulents. 
<br>The result is a battery-powered soil moisture monitor with a clear OLED display and ultra-low power sleep mode. 
Designed to be compact and dead-simple for plant lovers who just want to know when to water.
<br>This project features sensor calibration & analog input, power management, embedded UI design, and PCB-free low-power prototyping

# Project Goal
A compact, effective monitor for soil moisture coded in Arduino-style C++ with:
- A simple, low power display that shows
  - Moisture percentage
  - What to do with that info (Showing 'OK' or 'Needs water' depending on moisture)
  - A battery icon that conveys visually how much power remains
  - A percentage display cOnveying how much power remains
- A 3.7 V LiPo (charged via USB-C) for wireless power
- A sleep mode that saves battery after inactivity and powers down much of the internals
- A wake button to instantly get the onitor ready for work
- Accurate soil readings via capacitive sensor
- A tiny SoC to run the show (the Seeed Studio XIAO nRF52840 which is ~18mm X 21mm)
- Recharge capability (built into the nRF52840)

# Wiring

| Component                  | XIAO Pin          |
|---------------------------|-------------------|
| OLED (SSD1306, I2C)       | SDA → GPIO 4<br>SCL → GPIO 5 |
| Moisture Sensor           | A1 (Pin 1)        |
| Battery Voltage Divider   | A3 (Pin 3) via 100kΩ–100kΩ |
| Wake Button               | GPIO 2 → GND      |
| Battery                   | BAT+ pad + GND pin|

<img src="https://github.com/ChandlerEx/Projects/blob/891a760c9f960f25a479301b3d8ec1b8d20b8800/SoilMonitor/SoilMonWiring.png" alt="Wiring Diagram" width="250"/>

# Calibration
Moisture readings are calibrated between:
- *Dry:* `2665`
- *Wet:* `1155`
  
Mapped to 0–100% for display

# Enclosure
- Designed in Fusion 360 
- Custom 3D-printed shell with two bottom pieces that screw together and friction fit into the top piece
- Ground and power rails for easier soldering
- Holes with precise tolerances for display and wake button
<img src="https://github.com/ChandlerEx/Projects/blob/edbfac9c0c20d4b20371947c66a9b49363ebae98/SoilMonitor/SoilMonEnc.png" alt="Enclosure" width="250"/>

# Hardware
- XIAO nRF52840 SoC
- Liter 3.7V 400mAh 502035 LiPo Battery
- Stemedu Capacitive Analog Soil Moisture Sensor
- AITRIP 0.96 Inch OLED Display Module 12864
- Gebildet 7mm Black Prewired Mini Momentary Push Button,SPST
- Voltage Divider (2× 100kΩ - to scale battery voltage to safe ADC range)

# Firmware Logic Flow
- Startup
  - Initializes I2C display, ADCs, and button input
  - Loads sleep/reset logic
  - Reads initial moisture and battery values
- Active Mode (awake)
  - Refreshes OLED every 5 seconds
  - Displays current moisture %, status message, battery level
  - After 45 seconds of inactivity, powers down OLED and enters manual sleep
- Sleep Mode
  - Display powers down (SSD1306_DISPLAYOFF)
  - Device idles until wake button is pressed
- Wake Trigger
  - GPIO2 is pulled LOW via button → NVIC_SystemReset() executes
  - Full reboot ensures display and sensors reinitialize cleanly

# Photos
Top View
<img src="https://github.com/ChandlerEx/Projects/blob/501849a08d16480545f08d7e6b22d4a53f451889/SoilMonitor/SoilMonTopView.jpg" alt="Top View" width="250"/>

Side View
<img src="https://github.com/ChandlerEx/Projects/blob/501849a08d16480545f08d7e6b22d4a53f451889/SoilMonitor/SoilMonSideView.jpg" alt="Side View" width="250"/>

# Lessons Learned
- I was about 90% done when I realized I need this to be smaller and to be able to run without being plugged in - maybe next time figure that out before you've completed wiring and software
- Power and ground rails are a life saver on smaller projects like this
- Screw holes and snap fits have much lower tolerances in smaller projects
- Calibrating the sensor, and establishing good/bad ranges are sensor, environment, and plant dependent
- Soldering on such a tiny board drastically increases the likelihood of a short/frying it entirely

# Files
- Full ESP32 sketch: SoilMon.ino
- Full Fusion Archive: SoilMon.f3d
- STL files of all enclosure pieces: SoilMon.stl
- Video of demo: SoilMonDemo.mp4

# Potential Future Features
- A captive portal showing status of plant moisture (would need a different, likely larger chip for this)
- Monitoring and tracking data uploaded to a cloud hosted site (showing moisture status and last time it was watered)
- Menu of calibration options for the user (for dry/wet, and particular to the plant family)
- Further development on sleep mode, to drive down power usage 
- A rubber gasket to more securely hold the sensor in the enclousre, and prevent accidental water damage
