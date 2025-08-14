#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP_DoubleResetDetector.h>

#define DRD_TIMEOUT 3
#define DRD_ADDRESS 0

#define EEPROM_SIZE 64
#define TICKER_ADDR 0

#define LED_PIN D4
#define NUM_LEDS 7
Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void blinkYellowBlue(int times = 3);
void fadeToColor(uint32_t targetColor, int durationMs = 2000);

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

String symbol;
float lastChange = 0.0;
uint32_t currentColor = 0;
int currentPulseSpeed = 0;
unsigned long lastDataFetch = 0;
bool hasFetchedData = false;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  ring.begin();
  ring.setBrightness(60);

  // Boot color: solid yellow
  currentColor = ring.Color(255, 255, 0);
  solidColor(currentColor);

  WiFiManager wm;
  WiFiManagerParameter customTicker("ticker", "Stock Ticker (default: SPY)", "SPY", 10);
  wm.addParameter(&customTicker);

  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected - Starting WiFiManager");

    blinkYellowBlue();  // Reset feedback flash

    wm.resetSettings();
    wm.autoConnect("StockOrb_Setup");

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(100);
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFiManager exited, but failed to connect within 10s – entering blue error loop.");
      loopBlueError();
    }

    String newTicker = String(customTicker.getValue());
    saveTickerToEEPROM(newTicker);
    symbol = newTicker;
  } else {
    wm.autoConnect("StockOrb_Setup");
    unsigned long startAttempt = millis();
    // Wait up to 10 seconds for connection to take hold
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(100);
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFiManager exited, but failed to connect within 10s – entering blue error loop.");
      loopBlueError();
    }

    symbol = readTickerFromEEPROM();
    symbol.trim();

    if (symbol.length() == 0 || symbol == "\0" || !symbol.charAt(0)) {
      Serial.println("EEPROM ticker invalid, falling back to SPY");
      symbol = "SPY";
      saveTickerToEEPROM(symbol);
    }
  }

    // Confirm actual Wi-Fi connection
  if (WiFi.localIP().toString() == "0.0.0.0") {
      Serial.println("No valid IP assigned – entering blue error loop.");
      loopBlueError();
    }

  Serial.print("Final ticker: ");
  Serial.println(symbol);

  delay(1000);

  fetchStockData();
  lastDataFetch = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastDataFetch >= 30000) {
    fetchStockData();
    lastDataFetch = now;
  }

  updateLEDPulse();
}

void fetchStockData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=cvsnj4hr01qhup0ru5lgcvsnj4hr01qhup0ru5m0";
    WiFiClientSecure client;
    client.setInsecure();
    Serial.println("Fetching from URL:");
    Serial.println(url);
    http.begin(client, url);

    int httpCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Payload:");
      Serial.println(payload);
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float current = doc["c"];
        float prevClose = doc["pc"];
        lastChange = ((current - prevClose) / prevClose) * 100;

        Serial.printf("Market Change: %.2f%%\n", lastChange);

        uint32_t newColor = (lastChange >= 0) ? ring.Color(0, 255, 0) : ring.Color(255, 0, 0);

        if (!hasFetchedData) {
          fadeToColor(newColor, 2000);
          hasFetchedData = true;
        } else {
          currentColor = newColor;
        }

        float absChange = abs(lastChange);
        if (absChange >= 3.0) currentPulseSpeed = 30;
        else if (absChange >= 2.0) currentPulseSpeed = 50;
        else if (absChange >= 1.0) currentPulseSpeed = 80;
        else currentPulseSpeed = 0;
      } else {
        flashBlueError();
      }
    } else {
      flashBlueError();
    }

    http.end();
  } else {
    flashBlueError();
  }
}

void updateLEDPulse() {
  static unsigned long lastPulseUpdate = 0;
  static float timeOffset = 0.0;

  if (millis() - lastPulseUpdate >= 30) {
    lastPulseUpdate = millis();

    uint8_t r = (currentColor >> 16) & 0xFF;
    uint8_t g = (currentColor >> 8) & 0xFF;
    uint8_t b = currentColor & 0xFF;

    float brightnessFactor = 1.0;

    if (currentPulseSpeed > 0) {
      timeOffset += 0.05 * (100.0 / currentPulseSpeed);
      brightnessFactor = (sin(timeOffset) + 1.0) / 2.0;
      brightnessFactor = 0.5 + brightnessFactor * 0.5;
    }

    uint8_t r_adj = r * brightnessFactor;
    uint8_t g_adj = g * brightnessFactor;
    uint8_t b_adj = b * brightnessFactor;

    for (int i = 0; i < NUM_LEDS; i++) {
      ring.setPixelColor(i, r_adj, g_adj, b_adj);
    }
    ring.show();
  }
}

void solidColor(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, color);
  }
  ring.show();
}

void fadeToColor(uint32_t targetColor, int durationMs) {
  uint8_t r_start = (currentColor >> 16) & 0xFF;
  uint8_t g_start = (currentColor >> 8) & 0xFF;
  uint8_t b_start = currentColor & 0xFF;

  uint8_t r_end = (targetColor >> 16) & 0xFF;
  uint8_t g_end = (targetColor >> 8) & 0xFF;
  uint8_t b_end = targetColor & 0xFF;

  int steps = 50;
  for (int step = 0; step <= steps; step++) {
    float t = (float)step / steps;
    uint8_t r = r_start + t * (r_end - r_start);
    uint8_t g = g_start + t * (g_end - g_start);
    uint8_t b = b_start + t * (b_end - b_start);

    for (int i = 0; i < NUM_LEDS; i++) {
      ring.setPixelColor(i, r, g, b);
    }
    ring.show();
    delay(durationMs / steps);
  }

  currentColor = targetColor;
}

void flashBlueError() {
  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < NUM_LEDS; i++) ring.setPixelColor(i, 0, 0, 255);
    ring.show(); delay(500);
    ring.clear(); ring.show(); delay(500);
  }
}

void loopBlueError() {
  while (true) {
    Serial.println("Wi-Fi connection failed — blinking blue 10 times, will repeat.");
    for (int j = 0; j < 10; j++) {
      solidColor(ring.Color(0, 0, 255));  // Blue
      delay(300);
      ring.clear(); ring.show();
      delay(300);
    }
    delay(15000);  // Wait 15 seconds
  }
}

void blinkYellowBlue(int times) {
  for (int i = 0; i < times; i++) {
    solidColor(ring.Color(255, 255, 0));  // Yellow
    delay(300);
    solidColor(ring.Color(0, 0, 255));    // Blue
    delay(300);
  }
  solidColor(ring.Color(255, 255, 0));    // End on yellow
}

String readTickerFromEEPROM() {
  char ticker[11];
  for (int i = 0; i < 10; ++i) ticker[i] = EEPROM.read(TICKER_ADDR + i);
  ticker[10] = '\0';
  return String(ticker);
}

void saveTickerToEEPROM(String input) {
  String existing = readTickerFromEEPROM();
  if (input == existing) {
    Serial.println("Ticker unchanged, skipping EEPROM write.");
    return;
  }

  Serial.println("Updating ticker in EEPROM.");
  for (int i = 0; i < 9; ++i) {
    EEPROM.write(TICKER_ADDR + i, i < input.length() ? input[i] : 0);
  }
  EEPROM.commit();
}

