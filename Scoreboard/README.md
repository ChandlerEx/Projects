# Scoreboard
My wife and I are both Yankees fans, but often we'll be busy and unable to watch an entire game...or sometimes we just wanna know when first pitch is happening. Instead of using the computer or our phones, I thought it'd be nice to have a small, unobtrusive device that looks totally normal on an office desk, while still letting us know the latest on the Yankees.

So I bulit a device that looks and works like a normal clock, but if you touch the screen it shows the live, latest info from the Yankees game. The result: a live MLB scoreboard that displays Yankees game info on an ESP32 touchscreen (CYD) with clock, Wi-Fi config, and offline persistence.

# Project Goal
A wall-outlet powered ESP32 scoreboard that:
- Shows live Yankees game status
- Displays box scores (R / H / E)
- Displays current time with toggleable clock
- Is reversable, so it can angle the display at 30 or 60 degress
- Supports touchscreen interaction to switch views, toggle settings, and rest the stored wifi info

# Hardware
- ESP32-3248S035R (3.5" 320x480 TFT + resistive touch)
- Backlight dimming via GPIO21
- USB-powered; internal JST used for streamlined cable exit
- Enclosure custom designed from scratch and built with Fusion 360

# Architecture Overview
- Coded in Arduino-style C++
- Touch panel: Short tap toggles display mode (score ↔ clock), long-press enters settings
- Display modes:
    - MODE_SCOREBOARD – live game status, scores, inning, runners on base
    - MODE_CLOCK – digital clock + date
    - MODE_SETTINGS – flip screen, toggle clock, reset Wi-Fi
- Data source: Live API from statsapi.mlb.com
- Auto-reset: Refreshes at 3AM PT or after long uptime; boots in seconds
- Wi-Fi setup: Built-in captive portal (WiFiManager) on first boot or reset

# Photos
Early Proto<br>
<img src="https://github.com/user-attachments/assets/f6440fe7-e67e-47fb-aa2f-fecacd956c5a" alt="Early Prototype" width="250"/>

Finished Build<br>
<img src="https://github.com/user-attachments/assets/ccfb72b6-7670-484e-8638-d6ba8d4bcea1" alt="Finished Build" width="250"/>
<img src="https://github.com/ChandlerEx/Projects/blob/e6dc3a84d38a4de915836535537666dae7083257/Scoreboard/ScoreboardScore.jpg" alt="Showing Score" width="250"/>

Enclosure Design<br>
<img src="https://github.com/ChandlerEx/Projects/blob/399ff246cf15ed012120ca76c62e85f4193714b3/Scoreboard/ScoreEnc.png" alt="Enclosure Design" width="250"/>

# Lessons Learned
Time zones suck. Using UTC parsing + Pacific re-shifting was necessary.
Some Yankees games end with no bottom of 9th — needed a failsafe for final detection.
Display spacing and font choices were tuned for clarity on 240x320 screen.
Adding the JST wires inside for power made the enclosure much, much cleaner than using USB
Coding for the ESP32 is extremely touchy, especialy with fonts (FreeFont vs TextFont)
Making enclosure adjustments when the front plane is at a 60 deg angle is brutal

# Files
MLB StatsAPI: https://statsapi.mlb.com
Full ESP32 sketch: ScoreboardPrototype.ino
Full Fusion Archive: Scoreboard.f3d
STL files of enclosure and bezel: Scoreboard.stl

# Potential Future Features
Allow users to adjust brightness of the various displays in the settings and only with long or short screen taps
Allow users to follow any MLB team they want (configuring that in the CP when getting initially set up
Add other sports as well (NFL, NBA, MLS, NHL, F1)
Add a sneaky, unobtrusive icon to the clock screen to indicate a game is currently being played
