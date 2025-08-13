<img src="https://github.com/ChandlerEx/Projects/blob/2fbad740882c20011538793ed7427ab86f700809/TheOrb/OrbThumb.jpg" alt="The Orb Thumbnail" width="200"/>

# The Orb
For years I've had a fasination with an old concept - ambient computing. The idea is to convey digital information in a sensory way that doesn't 
rely on screens. I've had the idea in my head for as long as I can remember, but this pursuit provided an opportunity to bring it from brain to reality.
The result is this innocuous looking orb on a small pedestal that conveys with color and pulsing whether a certain financial index, commodity, stock 
or even crypto is up or down that day, and, by slowly pulsing the light, even conveys how drastic the change is.
<br>This project demonstrates:
- Custom PCB design
- Integration of threaded fits and screw fits
- Use of different kinds of filaments (PLA, PETG)
- Double Reset Detector logic

# Project Goal
A plug-and-play desktop stock indicator that:
- Shows market change percentage for a selected financial index, stock, or crypto holding via color and pulse speed
- Allows quick Wi-Fi setup via a captive portal
- Stores and remembers the selected ticker in EEPROM
- Handles connection failures gracefully with visual feedback
- Updates data every 30 seconds without needing user interaction

# Hardware
- Microcontroller: ESP8266 D1 Mini (LOLIN/WEMOS-compatible)
- LEDs: 7-LED WS2812B RGB ring
- Power: Micro-USB 5V
- Enclosure: 80mm glass dome with custom-printed base
- Extras:
        - 330 Ω resistor (in series from D4 → DIN)
        - 1000 µF capacitor (across VCC and GND at LED ring)

# Architecture Overview
- Language: Arduino-style C++
- Startup Behavior:

        Solid yellow boot color

        If double reset is detected, Wi-Fi settings reset and captive portal starts

        Captive portal allows user to enter:

            Wi-Fi SSID / password

            Stock ticker (defaults to SPY)

        Ticker is stored in EEPROM

    Data Fetch:

        HTTPS GET to Finnhub API every 30 seconds

        Calculates % change from previous close

    LED Logic:

        Green: positive change

        Red: negative change

        Pulse speed scales with volatility:

            ≥ 3%: very fast pulse

            ≥ 2%: fast pulse

            ≥ 1%: slow pulse

            < 1%: solid color

    Error Handling:

        Connection/data error: flashes blue

        Persistent Wi-Fi failure: blue error loop until reset

    Animations:

        First data fetch: fades from yellow to new color

        Subsequent updates: smooth brightness pulsing

Visual Feedback Reference
Event	LED Behavior
Boot	Solid yellow
Double reset detected	Yellow/blue alternating blinks
Positive market change	Solid/pulsing green
Negative market change	Solid/pulsing red
Wi-Fi/data error	Two quick blue flashes
Wi-Fi connection failed	Repeating 10× blue blink cycle
Lessons Learned

    Using ESP_DoubleResetDetector makes captive portal access easy without a physical button.

    WiFiClientSecure with setInsecure() is essential for quick HTTPS on ESP8266.

    EEPROM trimming and validation prevents storing corrupted tickers.

    Pulse animations look far smoother when brightness modulation is sinusoidal.

    A resistor + capacitor drastically improves LED stability and reduces flicker.

Files

    OrbCode_Final.ino — Full Arduino sketch

    StockOrb.f3d — Fusion 360 enclosure design

    StockOrb.stl — STL file for base/enclosure

    Finnhub API — Live stock data source

Potential Future Features

    User-selectable LED brightness in captive portal

    Multiple ticker rotation (e.g., SPY + a personal stock watchlist)

    Wi-Fi signal strength indicator via LED pattern

    Integration with cryptocurrency prices or market sentiment feeds

    Auto-off during non-market hours to save power

    MQTT/WebSocket mode for faster real-time updates

    Cloud logging of market data with historical trend playback

    E-paper or OLED module add-on for numeric display of price/percentage

    Configurable pulse styles (fade, strobe, breathing) per user preference

    Battery-powered portable version with sleep mode for travel use



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

