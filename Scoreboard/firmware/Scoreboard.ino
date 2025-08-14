// Scoreboard Prototype – Complete .INO

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>        // Captive-portal library
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_system.h>         // ESP.restart()
#include "driver/ledc.h"
#include <Preferences.h>        // Persistent storage for first-boot flag
#include <math.h>     // For sin, cos
#ifndef PI
  #define PI 3.14159265
#endif

// Touch panel pins
#define TOUCH_CS    33
#define TOUCH_IRQ   36
#define TOUCH_CLK   25
#define TOUCH_MOSI  32
#define TOUCH_MISO  12

// Backlight control pin
#define BACKLIGHT_PIN 21
#define DEBUG true

//Scoreboard Background Brightness
#define DIMSCORE 80  // Scoreboard Dimness

// Use default SPI bus for touch (letting TFT_eSPI handle SPI)
TFT_eSPI tft = TFT_eSPI();
Preferences prefs;
bool showClock = true;
bool flipped = false;

// Display modes
enum DisplayMode { MODE_SCOREBOARD, MODE_CLOCK, MODE_SETTINGS };
DisplayMode displayMode = MODE_CLOCK;
unsigned long lastTouch = 0;

int settingsModeState = 0;  // for settings screen - 0 = Flip, 1 = Clock, 2 = Wi-Fi reset
unsigned long touchStart = 0;
bool inLongPress = false;
DisplayMode prevModeBeforeSettings = MODE_CLOCK;

// Game state globals
String gamePk = "";
String awayAbbr = "???", homeAbbr = "???";
String gameState = "";
int awayR = 0, awayH = 0, awayE = 0;
int homeR = 0, homeH = 0, homeE = 0;
String inningState = "";
int inningNum = 0, outs = 0;
bool onFirst = false, onSecond = false, onThird = false;
time_t gameStartTime = 0, lastScheduleCheck = 0, resetTime = 0;
unsigned long lastPoll = 0;
const unsigned long POLL_INTERVAL = 15000;
const unsigned long RESET_INTERVAL = 6UL * 60UL * 60UL * 1000UL;
unsigned long bootTime = 0;

// Logging helper
void log(const char* msg) { if (DEBUG) Serial.println(msg); }

// Team ID → 3-letter abbreviation
String getTeamAbbr(int id) {
  switch (id) {
    case 147: return "NYY";  case 133: return "OAK";
    case 110: return "BAL";  case 111: return "BOS";
    case 145: return "CHW";  case 114: return "CLE";
    case 116: return "DET";  case 118: return "KC";
    case 108: return "LAA";  case 140: return "TEX";
    case 141: return "TOR";  case 139: return "TB";
    case 117: return "HOU";  case 146: return "MIA";
    case 121: return "NYM";  case 113: return "CIN";
    case 115: return "COL";  case 112: return "CHC";
    case 109: return "ARI";  case 119: return "LAD";
    case 134: return "MIL";  case 120: return "WSH";
    case 143: return "PHI";  case 135: return "SD";
    case 144: return "ATL";  case 142: return "MIN";
    case 138: return "STL";  case 137: return "SF";
    case 136: return "SEA";  case 158: return "PIT";
    default: return "???";
  }
}

// Draw on-base diamond graphic
void drawDiamondGraphic(int cx, int cy, int size) {
  int half = size / 2;
  tft.drawLine(cx, cy - half, cx + half, cy, TFT_BLACK);
  tft.drawLine(cx + half, cy, cx, cy + half, TFT_BLACK);
  tft.drawLine(cx, cy + half, cx - half, cy, TFT_BLACK);
  tft.drawLine(cx - half, cy, cx, cy - half, TFT_BLACK);
  if (onFirst)  tft.fillTriangle(cx + half, cy, cx + half - 6, cy - 6, cx + half - 6, cy + 6, TFT_BLACK);
  if (onSecond) tft.fillTriangle(cx, cy - half, cx - 6, cy - half + 6, cx + 6, cy - half + 6, TFT_BLACK);
  if (onThird)  tft.fillTriangle(cx - half, cy, cx - half + 6, cy - 6, cx - half + 6, cy + 6, TFT_BLACK);
}

// Draw inning and outs text
void drawGameStatus(int x, int y) {
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setCursor(x, y);
  if ((gameState == "Live" || gameState == "Final") && inningState != "" && inningNum > 0) {
    tft.printf("%s of %d, %d out", inningState.c_str(), inningNum, outs);
  }
}

// Render box score screen
void drawScoreboard() {
  analogWrite(BACKLIGHT_PIN, DIMSCORE);  // dim
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  // Headers R/H/E
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setCursor(180, 30); tft.print("R");
  tft.setCursor(230, 30); tft.print("H");
  tft.setCursor(280, 30); tft.print("E");
  // Teams
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.setCursor(10, 80);  tft.print(awayAbbr + (inningState == "Top" ? " *" : ""));
  tft.setCursor(10, 130); tft.print(homeAbbr + (inningState == "Bottom" ? " *" : ""));
  // Scores
  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.setCursor(180, 80);  tft.printf("%d", awayR);
  tft.setCursor(230, 80);  tft.printf("%d", awayH);
  tft.setCursor(280, 80);  tft.printf("%d", awayE);
  tft.setCursor(180, 130); tft.printf("%d", homeR);
  tft.setCursor(230, 130); tft.printf("%d", homeH);
  tft.setCursor(280, 130); tft.printf("%d", homeE);
  // Footer message
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setCursor(10, 200);
  if (gamePk == "") tft.print("No Yankees game today.");
  else if (gameState == "Final") tft.print("Final Score");
  else if (gameState == "Live" && inningState != "" && inningNum > 0) {
    drawGameStatus(10, 200);
    drawDiamondGraphic(260, 200, 30);
  } else {
    time_t now = time(nullptr);
    int diff = gameStartTime - now;
    if (diff > 0) {
      int mins = diff / 60;
      tft.printf("First pitch in %dh %dm", mins / 60, mins % 60);
    } else {
      tft.print("Game not started");
    }
  }
}

void drawBetterGearIcon(int cx, int cy, int r) {
  // Draw outer gear teeth (boxy stubs)
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4.0;
    int x = cx + cos(angle) * (r + 2);
    int y = cy + sin(angle) * (r + 2);
    tft.fillRect(x - 1, y - 1, 3, 3, TFT_LIGHTGREY);
  }

  // Draw gear body (ring)
  tft.fillCircle(cx, cy, r, TFT_LIGHTGREY);
  tft.fillCircle(cx, cy, r - 4, TFT_BLACK);  // hollow center ring

  // Center hole
  tft.fillCircle(cx, cy, 2, TFT_LIGHTGREY);
}

// Render clock/date screen
void drawClockScreen() {
  analogWrite(BACKLIGHT_PIN, 255); // full brightness for clock
  tft.fillScreen(TFT_BLACK);

  // Get local time
  time_t now = time(nullptr);
  struct tm ti;
  localtime_r(&now, &ti);

  // Format time: "HH:MM AM/PM"
  char timeBuf[11];
  strftime(timeBuf, sizeof(timeBuf), "%I:%M %p", &ti);

  // Strip leading space if hour < 10
  if (timeBuf[0] == ' ') {
    memmove(timeBuf, timeBuf + 1, strlen(timeBuf));
  }

  // Time settings: red 7-seg font, centered
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextFont(7);     // 7-segment style
  tft.setTextSize(2);     // make it big
  tft.setTextDatum(MC_DATUM);  // center anchor
  tft.drawString(timeBuf, (tft.width() / 2) + 34, tft.height() / 2);  // ← THIS is what was missing

  // Format date: "Tuesday, July 22, 2025"
  char dateBuf[32];
  strftime(dateBuf, sizeof(dateBuf), "%A, %B %d, %Y", &ti);

  // Date settings: white small font, left-bottom
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);     // small proportional
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);  // top-left anchor
  tft.drawString(dateBuf, 10, tft.height() - 20);

  // Draw gear icon on bottom right
  drawBetterGearIcon(tft.width() - 20, tft.height() - 20, 10);
}

void drawSettingsScreen() {
  analogWrite(BACKLIGHT_PIN, 255);  // full brightness
  tft.fillScreen(TFT_BLACK);

  // Header: “Settings”
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Settings", tft.width() / 2, 40);

  // Body text
  tft.setFreeFont(&FreeSans12pt7b);
  tft.setTextDatum(TC_DATUM);

  switch (settingsModeState) {
    case 0:
      tft.drawString("Flip Display", tft.width() / 2, 100);
      tft.drawString("(Hold to flip)", tft.width() / 2, 140);
      break;
    case 1:
      tft.drawString("Clock Display", tft.width() / 2, 100);
      tft.drawString(String("Currently: ") + (showClock ? "ON" : "OFF"), tft.width() / 2, 140);
      tft.drawString("(Hold to toggle)", tft.width() / 2, 180);
      break;
    case 2:
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("Reset Wi-Fi", tft.width() / 2, 100);
      tft.drawString("Hold 5 sec to reset", tft.width() / 2, 140);
      break;
    case 3:
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Return to Main", tft.width() / 2, 100);
      tft.drawString("(Hold to exit)", tft.width() / 2, 140);
      break;
  }
}

// Fetch today’s schedule
void fetchSchedule() {
  HTTPClient http;
  http.begin("https://statsapi.mlb.com/api/v1/schedule?sportId=1&teamId=147");
  if (http.GET() == 200) {
    DynamicJsonDocument doc(8192);
    if (!deserializeJson(doc, http.getStream())) {
      JsonArray dates = doc["dates"].as<JsonArray>();
      if (dates.size() > 0 && dates[0]["games"].size() > 0) {
        JsonObject g = dates[0]["games"][0].as<JsonObject>();
        gamePk = String((int)g["gamePk"]);
        gameState = g["status"]["abstractGameState"] | "";
        homeAbbr = getTeamAbbr((int)g["teams"]["home"]["team"]["id"]);
        awayAbbr = getTeamAbbr((int)g["teams"]["away"]["team"]["id"]);
        String gd = g["gameDate"] | "";
        if (gd != "") {
          struct tm tm = {};
          strptime(gd.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);

          setenv("TZ", "UTC0", 1);  // Interpret tm as UTC
          tzset();
          time_t utc = mktime(&tm);
          setenv("TZ", "PST8PDT", 1);  // Restore local zone
          tzset();

          gameStartTime = utc;

          time_t cur = time(nullptr);
          struct tm lt = *localtime(&cur);
          lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
          time_t midnight = mktime(&lt) + 86400;

          time_t post = gameStartTime;
          resetTime = max(midnight, post);
        }

      } else {
        gamePk = ""; awayAbbr = "OFF"; homeAbbr = "DAY";
      }
    }
  }
  http.end();
}

// Fetch live box score
void fetchLinescore() {
  if (gamePk == "") return;
  HTTPClient http;
  http.begin("https://statsapi.mlb.com/api/v1/game/" + gamePk + "/linescore");
  if (http.GET() == 200) {
    DynamicJsonDocument doc(8192);
    if (!deserializeJson(doc, http.getStream())) {
      awayR = doc["teams"]["away"]["runs"] | 0;
      awayH = doc["teams"]["away"]["hits"] | 0;
      awayE = doc["teams"]["away"]["errors"] | 0;
      homeR = doc["teams"]["home"]["runs"] | 0;
      homeH = doc["teams"]["home"]["hits"] | 0;
      homeE = doc["teams"]["home"]["errors"] | 0;
      inningState = doc["inningState"] | "";
      inningNum = doc["currentInning"] | 0;
      outs = doc["outs"] | 0;
      JsonObject of = doc["offense"].as<JsonObject>();
      onFirst = of.containsKey("first");
      onSecond = of.containsKey("second");
      onThird = of.containsKey("third");
    }
  }
  http.end();
}

// Refresh only gameState
void refreshGameState() {
  if (gamePk == "") return;
  HTTPClient http;
  http.begin("https://statsapi.mlb.com/api/v1/schedule?sportId=1&teamId=147");
  if (http.GET() == 200) {
    DynamicJsonDocument doc(4096);
    if (!deserializeJson(doc, http.getStream())) {
      JsonArray dates = doc["dates"].as<JsonArray>();
      if (dates.size() > 0 && dates[0]["games"].size() > 0) {
        gameState = dates[0]["games"][0]["status"]["abstractGameState"] | gameState;
      }
    }
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Backlight
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, 255);  // full brightness

  // Touch IRQ as simple button input (fallback)
  pinMode(TOUCH_IRQ, INPUT_PULLUP);

  // Begin screen
  tft.init();

  // Preferences and screen flip
  prefs.begin("cfg", false);
  bool firstBoot = !prefs.getBool("configured", false);
  showClock = prefs.getBool("clock", true);
  flipped = prefs.getBool("flipped", false);
  prefs.end();

  tft.setRotation(flipped ? 3 : 1);

  // WiFi config
  WiFiManager wm;
  if (firstBoot) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(4); // FIRST CHANGE
    tft.setCursor(10, 40);
    tft.println("          TO GET STARTED:");
    tft.println("");
    tft.println("* Join Wi-Fi: 'Scoreboard'");
    tft.println("* Go to 192.168.4.1");
    tft.println("* Follow prompts");
    delay(3000);
    wm.resetSettings();
    wm.setConfigPortalTimeout(180);
    if (!wm.autoConnect("Scoreboard")) ESP.restart();
    prefs.begin("cfg", false);
    prefs.putBool("configured", true);
    prefs.end();
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    unsigned long st = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - st < 10000) delay(500);
    if (WiFi.status() != WL_CONNECTED) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextFont(4);
      tft.setCursor(10, 40);
      tft.println("          TO GET STARTED:");
      tft.println("");
      tft.println("* Join Wi-Fi: 'Scoreboard'");
      tft.println("* Go to 192.168.4.1");
      tft.println("* Follow prompts");
      delay(3000);
      wm.setConfigPortalTimeout(180);
      if (!wm.startConfigPortal("Scoreboard")) ESP.restart();
    }
  }

  // Time sync (Pacific)
  configTime(-7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) delay(100);

  // Initial data pull
  fetchSchedule();
  fetchLinescore();

  // Draw initial screen
  if (displayMode == MODE_SCOREBOARD) drawScoreboard();
  else drawClockScreen();

  bootTime = millis();
}

void loop() {
  // --- 1) Serial “reset” command ---
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "reset") {
      prefs.begin("cfg", false);
      prefs.clear();
      prefs.end();
      Serial.println("⚠️ Credentials cleared – rebooting");
      ESP.restart();
    }
  }

  // --- 2) IRQ-only tap to cycle modes ---
  static bool lastIRQ = HIGH;
  bool currIRQ = digitalRead(TOUCH_IRQ);

  if (lastIRQ == HIGH && currIRQ == LOW) {
    touchStart = millis();
    inLongPress = true;
  }
  else if (lastIRQ == LOW && currIRQ == HIGH && inLongPress) {
    unsigned long pressDuration = millis() - touchStart;
    inLongPress = false;

    // ---------- Settings Mode ----------
    if (displayMode == MODE_SETTINGS) {
      switch (settingsModeState) {
        case 0:  // Flip
          if (pressDuration > 1000) {
            flipped = !flipped;
            prefs.begin("cfg", false);
            prefs.putBool("flipped", flipped);
            prefs.end();
            tft.setRotation(flipped ? 3 : 1);
          }
          break;

        case 1:  // Clock toggle
          if (pressDuration > 1000) {
            showClock = !showClock;
            prefs.begin("cfg", false);
            prefs.putBool("clock", showClock);
            prefs.end();

            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setFreeFont(&FreeSans12pt7b);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(String("Clock: ") + (showClock ? "ON" : "OFF"), tft.width() / 2, tft.height() / 2);

            delay(1500);  // Show feedback
          }
          break;

        case 2:  // Wi-Fi reset
          if (pressDuration > 5000) {
            prefs.begin("cfg", false);
            prefs.clear();
            prefs.end();
            Serial.println("⚠️ Wi-Fi credentials cleared. Restarting...");
            ESP.restart();
          }
          break;

        case 3:  // Return to main
          if (pressDuration > 1000) {
            displayMode = prevModeBeforeSettings;
            prefs.begin("cfg", false);
            showClock = prefs.getBool("clock", true);
            prefs.end();
            if (displayMode == MODE_CLOCK) drawClockScreen();
            else drawScoreboard();
            settingsModeState = 0;
            return;
          }
          break;
      }

      // Advance to next setting after any press
      settingsModeState = (settingsModeState + 1) % 4;
      drawSettingsScreen();
    }

    // ---------- Clock or Scoreboard Mode ----------
    else {
      if (pressDuration > 1500) {
        // Long press → enter settings
        prevModeBeforeSettings = displayMode;
        displayMode = MODE_SETTINGS;
        settingsModeState = 0;
        drawSettingsScreen();
      } else {
        // Short press → toggle modes, skip clock if disabled
        if (displayMode == MODE_SCOREBOARD && showClock) {
          displayMode = MODE_CLOCK;
          drawClockScreen();
        } else {
          displayMode = MODE_SCOREBOARD;
          drawScoreboard();
        }
      }
    }
  }
  lastIRQ = currIRQ;

  // --- 3) Clock refresh on change ---
  static unsigned long lastClockCheck = 0;
  static String lastTimeStr = "";
  if (displayMode == MODE_CLOCK && millis() - lastClockCheck > 250) {
    lastClockCheck = millis();
    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);
    char buf[11];
    strftime(buf, sizeof(buf), "%I:%M %p", &ti);
    if (buf[0] == ' ') memmove(buf, buf + 1, strlen(buf));
    String currentTimeStr(buf);
    if (currentTimeStr != lastTimeStr) {
      drawClockScreen();
      lastTimeStr = currentTimeStr;
    }
  }

  // --- 4) Scoreboard polling ---
  if (displayMode == MODE_SCOREBOARD) {
    if (millis() - bootTime > RESET_INTERVAL) {
      ESP.restart();
    }

    time_t now = time(nullptr);
    if (now > resetTime && lastScheduleCheck < resetTime) {
      fetchSchedule();
      fetchLinescore();
      drawScoreboard();
      lastScheduleCheck = now;
    }

    if (millis() - lastPoll > POLL_INTERVAL) {
      refreshGameState();
      fetchLinescore();
      drawScoreboard();
      lastPoll = millis();
    }
  }

  // --- 5) Daily 3AM refresh (regardless of mode) ---
  time_t now = time(nullptr);
  struct tm ti;
  localtime_r(&now, &ti);
  if (ti.tm_hour == 3 && ti.tm_min == 0 && lastScheduleCheck != now) {
    fetchSchedule();
    fetchLinescore();
    lastScheduleCheck = now;
  }
}
