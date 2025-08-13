<img src="https://github.com/ChandlerEx/Projects/blob/2fbad740882c20011538793ed7427ab86f700809/TheOrb/OrbThumb.jpg" alt="The Orb Thumbnail" width="200"/>

# Soil Monitor
My wife got a succulent as a gift, but she was worried about overwatering it - a common way to kill a succulent! So, to help, I decided to build a 
soil monitor and configure it specifically for succulents. 
<br>The result is a battery-powered soil moisture monitor with a clear OLED display and ultra-low power sleep mode. 
Designed to be compact and dead-simple for plant lovers who just want to know when to water.
<br>This project demonstrates:
- sensor calibration & analog input
- power management
- embedded UI design
- PCB-free low-power prototyping

# Project Goal
A compact, effective monitor for soil moisture coded in Arduino-style C++ with:
- A simple, low power display that shows
  - Moisture percentage
  - What to do with that info (Showing 'OK' or 'Needs water' depending on moisture)
  - A battery icon that conveys visually how much power remains
  - A percentage display conveying how much power remains
- A 3.7 V LiPo (charged via USB-C) for wireless power
- A sleep mode that saves battery after inactivity and powers down much of the internals
- A wake button to rapidly get the monitor ready for work
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

<img src="https://github.com/ChandlerEx/Projects/blob/9a11e649d50762af672b37ba1d7330335580c16b/SoilMonitor/SoilMonBreadboard.jpg" alt="Soil Monitor Breadboard" width="300"/>

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
Top View<br>
<img src="https://github.com/ChandlerEx/Projects/blob/501849a08d16480545f08d7e6b22d4a53f451889/SoilMonitor/SoilMonTopView.jpg" alt="Top View" width="250"/>

Side View<br>
<img src="https://github.com/ChandlerEx/Projects/blob/501849a08d16480545f08d7e6b22d4a53f451889/SoilMonitor/SoilMonSideView.jpg" alt="Side View" width="250"/>

# Lessons Learned
- Define power & size constraints before wiring & coding
- Power/ground rails simplify small builds
- Small enclosures need tighter tolerance testing
- Moisture thresholds are plant & environment specific
- Soldering on tiny boards increases short-circuit risk

# Files
- SoilMon.ino -- Full Arduino sketch
- SoilMon.f3d -- Fusion 360 source
- SoilMon.stl -- STL file for printing
- SoilMonDemo.mp4 -- Video of demo

# Potential Future Features
- Provide a broader set of inital indicators to track on WiFi setup screen
- Make brightness easily adjustable (on initial config, or with a light sensor to 'read the room'
- Add small screen to front of base showing indicator and current % change, possibly allowing multiple indicator tracking
- Provide multiple glass orb designs (clear glass with design laser etched inside? Sqishy orbs? Logo-covered orbs?)





# Quick start (new user workflow)

1. **Power up and join setup Wi-Fi**
* On boot the firmware starts WiFiManager and opens an AP named **`Orb_Setup`** for provisioning. Connect to that network
* to enter your Wi-Fi credentials and the **“Stock Ticker (default: SPY)”** field. &#x20;

2. **(Optional) Reconfigure later**
* Performing a **double-reset** triggers configuration mode: settings are cleared, the `StockOrb_Setup` portal opens again,
* and the entered ticker is saved. (DoubleResetDetector timeout is set to 3.) &#x20;

3. **Ticker storage & fallback**
* The selected ticker is persisted to EEPROM; on normal boots the orb uses the saved value. If the stored value is
* invalid, it **falls back to `SPY`**. &#x20;

4. **First data pull**
* After setup, the orb **immediately fetches** market data once and logs the final ticker to Serial.&#x20;

5. **Ongoing updates**
* The orb refreshes data **every 30 seconds** and animates continuously.&#x20;
  
6. **What the lights mean**
* **Green** = positive change
* **Red** = negative change
* * **Solid Yellow** = Awaiting wifi info, typically on first start-up
* *NOTE* Pulse speed increases with magnitude (≥1%, ≥2%, ≥3% tiers)
* **Two blue flashes** indicate a network/parse failure.&#x20;


# Feature list

* **Target & LED hardware**
  * Uses an ESP8266 (D1 Mini) with an Adafruit NeoPixel and a ring of 7 LEDs on pin D4 (GRB, 800 kHz). Brightness is set to 60 at startup. &#x20;

* **Wi-Fi provisioning & reconfiguration**
  * WiFiManager portal **`Orb_Setup`** for first-time setup and later changes;
  * includes a custom parameter for Stock Ticker (default: "SPY", to track the S&P500) Can also be used to track crypto, Individual stocks, even commodities
  * DoubleResetDetector (using button on back of base) puts the device back into setup mode and clears saved settings.  &#x20;

* **Persistent configuration**
  * EEPROM initialized to 64 bytes; ticker stored starting at address **0** via `saveTickerToEEPROM()` / `readTickerFromEEPROM()`.
  * Invalid storage auto-reverts to **SPY**. (Save loop writes up to **9 characters**.)   &#x20;

* **Data source & security**
  * Fetches quotes from Finnhub’s `/api/v1/quote` endpoint using HTTPS via `WiFiClientSecure` (certificate checking disabled with `setInsecure()`), and `ESP8266HTTPClient`. &#x20;

* **Update cadence**
  * Immediate fetch on boot, then 30-second polling loop. &#x20;

* **Parsing & signal calculation**
  * Parses JSON fields `c` (current) and `pc` (previous close), computes percent change `(c - pc) / pc * 100`.&#x20;

* **Color logic**
  * **Green** for ≥0% change; **Red** for <0%.&#x20;

* **Magnitude-based pulsing**
  * Pulse speed scales with absolute change: ≥3% → 30, ≥2% → 50, ≥1% → 80, else solid.
  * Animation uses a sine-based brightness wave clamped to 50–100% for a smooth breathing effect. &#x20;

* **Error feedback**
  * Any HTTP/connection/parse failure calls `flashBlueError()` which blinks blue twice. &#x20;

* **Serial diagnostics**
  * Verbose logs include the final ticker, request URL, HTTP status, payload, and computed market change %.   &#x20;

