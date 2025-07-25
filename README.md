# Scoreboard
My wife and I are both Yankees fans, but often people at work are discouraged from looking up sports scores. So I bulit a device that looks and works like a normal clock, but if you touch the screen it shows the live, latest info from the Yankees game. The result: a live MLB scoreboard that displays Yankees game info on an ESP32 touchscreen (CYD) with clock, Wi-Fi config, and offline persistence.

# Project Goal
A wall-powered ESP32 scoreboard that:
    Shows live Yankees game status
    Displays box scores (R / H / E)
    Displays current time with toggleable clock
    Supports touchscreen interaction to switch views, toggle settings, and rest the stored wifi info

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
