# Scoreboard
**[One-Sheet PDF – Quick Build Reference](https://raw.githubusercontent.com/ChandlerEx/Projects/main/Scoreboard/docs/Scoreboard_One_Sheet.pdf)**  
*(Overview, dimensions, wiring, build steps, and lessons learned)*

Video demonstration - click thumbnail below:<br>
[![Scoreboard Demo Video](https://img.youtube.com/vi/gKBhKu_5G0A/0.jpg)](https://www.youtube.com/watch?v=gKBhKu_5G0A)

## Overview
The Scoreboard is a desk-friendly digital clock with a hidden touch interface that displays live Yankees game data on demand. Tapping the screen queries the MLB 
StatsAPI to show scores, inning status, and base runners; long-pressing opens settings for screen flipping, clock toggle, and Wi-Fi reset. Supports quick Wi-Fi 
setup via a captive portal and stores data for offline viewing. Measures 98 × 60 × 57 mm (124 g). Built around an ESP32 CYD touchscreen with a custom 3D-printed enclosure modeled in Fusion 360.

## Photos
**Early Prototype** (Used off-the-shelf enclosure)<br>
<img src="https://github.com/user-attachments/assets/f6440fe7-e67e-47fb-aa2f-fecacd956c5a" alt="Early Prototype" height="200"/>

**Finished Build**<br>
<img src="https://github.com/ChandlerEx/Projects/blob/e6dc3a84d38a4de915836535537666dae7083257/Scoreboard/ScoreboardScore.jpg" alt="Showing Score" height="200"/>
<img src="https://github.com/ChandlerEx/Projects/blob/901382017dd94e5ac3727795fe7eb9a30eb157ab/Scoreboard/ScoreboardSettings.jpg" alt="ScoreboardSettings" height="200"/>
<img src="https://github.com/ChandlerEx/Projects/blob/da597a4e2234c74e041e07c38a4a65b5bd89d893/Scoreboard/ScoreboardInUse.jpg" alt="ScoreboardInUse" height="200"/>  

**Enclosure Design**<br>
<img src="https://github.com/ChandlerEx/Projects/blob/f431f446cbc0d26ab5680d121ebbc427395d03b7/Scoreboard/ScoreboardEncExploded.png" alt="Showing Score" height="200"/>

## Bill of Materials
| Item | Description | Notes |
|------|-------------|-------|
| ESP32 CYD Board (2.4" Display) | All-in-one ESP32 microcontroller with integrated capacitive touch TFT display | Main processing unit and display; USB powered |
| Custom 3D-Printed Enclosure | Two-part design with base and snap-fit bezel | Printed in PLA; parametric CAD model in Fusion 360 |
| Screws (M3 x 8mm) | Machine screws for mounting board to enclosure | Qty: 4 |
| USB Cable (Power leads to USB-A) | Power cable wired into JST port on CYD | Routed through rear port |
| Filament (PLA) | Material for enclosure | Color as desired - Used Yankees Blue here |
| Adhesive Rubber Feet | Non-slip feet for enclosure stability | Qty: 4 (optional)|

## Build Steps
- Upload 'ScoreboardPrototype.ino' to the ESP32 embedded in the CYD module
- Configure Wi-Fi access via the Captive portal
- Print the bezel and base from the 'Scoreboard.stl' file in this project
- Route the power and ground leads from a USB-A cable through the port on back, tie a knot into the USB cable for strain relief, then wire into the JST port
- Screw the CYD onto the base the snap fit the bezel onto the front

## Code
- Plug in the 2.4" CYD with a micro-USB to USB-A cable, then open Arduino IDE
- Select 'ESP32 Dev Module' and the relevant USB port
- Upload the 'ScoreboardPrototype.ino' found in these project files to the ESP32 embedded in the CYD module
- Watch Serial monitor for errors, then test functionality on the board to confirm all went well

## Lessons Learned
- Some Yankees games end with no bottom of 9th — needed a failsafe for final detection.
- Display spacing and font choices are tricky on this module, and need to be tuned for clarity.
- Adding the JST wires inside for power (instead of using the micro-USB port) made the enclosure much smaller
- Making enclosure adjustments in Fusion 360 is brutal when the front plane is at a 60 deg angle

## License
MIT license

---
<details>
<summary>Click here for more background and technical details about Scoreboard</summary>

# More Details

## Background
I live in a New York Yankees supporting household, but often we'll be busy and unable to watch an entire game. Sometimes we just wanna know when first pitch is happening or what the score is without having to tune in or check our phones.<br>

The initial idea was a module that plugged into the wall, stayed at the wall outlet, and made a sound if there was a homerun or the Yankees won. That evolved as I realized how intrusive that could be, especially in a home office. Then, I thought, maybe it'd be nice to have a small, unobtrusive device that looks totally normal on an office desk, while still letting us know the latest on the Yankees.<br>

This device looks and works like a normal clock, but touching the screen on the prototype shows live info from the Yankees game. The result: a live MLB scoreboard that displays Yankees game info on an ESP32 touchscreen (CYD) with clock, Wi-Fi config, and offline persistence.

## Project Goal
A wall-outlet powered ESP32 scoreboard that:
- Shows live Yankees game status
- Displays box scores (R / H / E)
- Displays current time with toggleable clock
- Is reversible, so it can angle the display at 30 or 60 degress
- Supports touchscreen interaction to switch views, toggle settings, and reset the stored Wi-Fi info

## Hardware
- ESP32-3248S035R (TFT + resistive touch)
- Backlight dimming via GPIO21
- USB-powered; internal JST used for streamlined cable exit
- Enclosure custom designed from scratch and built with Fusion 360

## Architecture Overview
- Coded in Arduino-style C++
- Touch panel: Short tap toggles display mode (score ↔ clock), long-press enters settings
- Display modes:
    - MODE_SCOREBOARD – live game status, scores, inning, runners on base
    - MODE_CLOCK – digital clock + date
    - MODE_SETTINGS – flip screen, toggle clock, reset Wi-Fi
- Data source: Live API from statsapi.mlb.com
- Auto-reset: Refreshes at 3AM PT or after long uptime; boots in seconds
- Wi-Fi setup: Built-in captive portal (WiFiManager) on first boot or reset

## Files
- MLB StatsAPI: https://statsapi.mlb.com
- Full ESP32 sketch: ScoreboardPrototype.ino
- Full Fusion Archive: Scoreboard.f3d
- STL files of enclosure and bezel: Scoreboard.stl

## Potential Future Features
- Allow users to adjust brightness of the various displays in the settings and only with long or short screen taps
- Allow users to follow any MLB team they want (configuring that in the CP when getting initially set up)
- Add other sports as well (NFL, NBA, MLS, NHL, F1)
- Add a sneaky, unobtrusive icon to the clock screen to indicate a game is currently being played
</details>
