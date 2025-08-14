//SoilMon working code 

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === Display Configuration ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // No reset pin used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
extern void ssd1306_command(uint8_t c);  // Needed to call internal command

// === Pin Assignments (based on silkscreen labels) ===
#define MOISTURE_PIN     A1      // Pin labeled "1" (P0.03)
#define WAKE_BUTTON_PIN  2       // Pin labeled "2" (P0.28)
#define BATTERY_PIN      A3      // Pin labeled "3" (P0.04)
#define LED_RED          11      // P1.11 (may not be functional)
#define LED_GREEN        13      // P1.13
#define LED_BLUE         14      // P1.14

// === Moisture Calibration ===
#define DRY_READING 690
#define WET_READING 310
#define MOISTURE_OK_THRESHOLD 20  // in percent

// === Timing ===
unsigned long displayStartTime = 0;
const unsigned long DISPLAY_DURATION_MS = 45000;  // 45 secs

// === LED Helper ===
void setRGB(bool red, bool green, bool blue) {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, red ? LOW : HIGH);
  digitalWrite(LED_GREEN, green ? LOW : HIGH);
  digitalWrite(LED_BLUE, blue ? LOW : HIGH);
}

// === Moisture Sensor Reading ===
int readMoisturePercent() {
  int raw = analogRead(MOISTURE_PIN);
  int percent = map(raw, DRY_READING, WET_READING, 0, 100);
  return constrain(percent, 0, 100);
}

// === Battery Voltage Helpers ===
float readBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  float voltage = (raw / 1023.0) * 3.3;     // ADC result to volts
  return voltage * 2;  // Account for 2:1 divider
}

int batteryPercent(float voltage) {
  // Map ~3.0V–4.2V range to 0–100%
  int percent = map(voltage * 100, 300, 420, 0, 100);  // Multiplied to avoid float issues
  return constrain(percent, 0, 100);
}

// === OLED Display Logic ===
void displayStatus(int moisture) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);

  if (moisture <= MOISTURE_OK_THRESHOLD) {
    display.println("Water,");
    display.println("please!");
    setRGB(true, false, false);  // Red
  } else {
    display.println("");
    display.println("OK :)");
    setRGB(false, true, false);  // Green
  }

  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Moisture: ");
  display.print(moisture);
  display.println("%");

  float batteryV = readBatteryVoltage();
  int batteryP = batteryPercent(batteryV);
  drawBatteryIcon(batteryP);

  display.display();
}

void drawBatteryIcon(int percent) {
  int x = 100, y = 0;  // Top-right corner
  int w = 24, h = 10;

  display.drawRect(x, y, w, h, SSD1306_WHITE);         // Battery body
  display.drawRect(x + w, y + 3, 2, 4, SSD1306_WHITE); // Battery tip

  int fillWidth = map(percent, 0, 100, 0, w - 2);
  display.fillRect(x + 1, y + 1, fillWidth, h - 2, SSD1306_WHITE);

  display.setCursor(x, y + h + 1);
  display.setTextSize(1);
  display.print(percent);
  display.println("%");
}

// === Sleep Routine ===
void goToSleep() {
  setRGB(false, false, false);
  display.clearDisplay();
  display.display();

  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);

  // Wait for button press (active LOW)
  while (digitalRead(WAKE_BUTTON_PIN) == HIGH) {
    delay(50);
  }

  // Debounce hold
  delay(500);

  // === OLED power down ===
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  // Soft reset the system (works reliably on this board)
  NVIC_SystemReset();
}

// === Setup ===
void setup() {
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOISTURE_PIN, INPUT);

  Wire.begin();  // SDA = pin 4, SCL = pin 5 (hardware defaults)

  delay(200);  // I2C settle time

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Silent fail
    while (true) { delay(1); }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Soil Monitor");
  display.println("Waking up...");
  display.display();

  delay(500);

  int moisture = readMoisturePercent();
  displayStatus(moisture);
  displayStartTime = millis();
}

// === Main Loop ===
void loop() {
  if (millis() - displayStartTime > DISPLAY_DURATION_MS) {
    goToSleep();
  }

  int moisture = readMoisturePercent();
  displayStatus(moisture);
  delay(5000);  // Update every 5s

  float batteryV = readBatteryVoltage();
  int batteryP = batteryPercent(batteryV);

  int raw = analogRead(MOISTURE_PIN);
  Serial.print("Raw Moisture: ");
  Serial.println(raw);

}
