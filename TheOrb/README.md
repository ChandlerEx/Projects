

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
* *NOTE* Pulse speed increases with magnitude (≥1%, ≥2%, ≥3% tiers)
* **Two blue flashes** indicate a network/parse failure.
* **Solid Yellow** = Awaiting wifi info &#x20;

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

