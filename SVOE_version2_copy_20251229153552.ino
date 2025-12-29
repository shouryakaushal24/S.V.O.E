
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>

// ---------------- WIFI ----------------
const char* ssid = "LAPTOPHOTSPOT2010";
const char* password = "401y1R@5";
const char* googleSheetLink = "https://script.google.com/macros/s/AKfycbz7WrvfwCT_2qKUbHzersLuzxmCyJmD48Aq9ieXO8DdM0QEjIKdPltPrb8IozhR9I4N/exec";

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- DHT22 ----------------
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------------- CO2 SENSOR (MH-Z19C) ----------------
#define RX_PIN 16
#define TX_PIN 17
HardwareSerial mhz19Serial(1);

// ---------------- LEDS ----------------
#define GREEN_LED 25
#define YELLOW_LED 26
#define RED_LED 27

// ---------------- THRESHOLDS ----------------
#define GOOD_MAX 800
#define MODERATE_MAX 1200

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastSheetSend = 0;
unsigned long lastWifiAttempt = 0;
const unsigned long sensorInterval = 3000;    // read sensors every 3s
const unsigned long sheetInterval = 60000;    // send to sheet every 60s
const unsigned long wifiReconnectInterval = 30000; // try reconnect every 30s
const unsigned long wifiConnectTimeout = 6000; // 6 seconds initial connect timeout

// Stored values
int co2Value = 0;
float temperature = 0;
float humidity = 0;

bool liveDisplay = false;

// MH-Z19C warmup: ignore first N readings (sensor needs time to stabilise)
const int WARMUP_DISCARD_READS = 6;
int warmupCounter = 0;

// ----------------------------------------------------
// CENTER TEXT FUNCTION
// ----------------------------------------------------
void centerText(const char* text, int size, int y) {
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, y);
  display.println(text);
}

// ----------------------------------------------------
// GOOGLE SHEETS
// ----------------------------------------------------
void sendToGoogleSheets(int co2, float temp, float hum) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(googleSheetLink) + "?co2=" + co2 + "&temp=" + temp + "&hum=" + hum;
    http.begin(url);
    int code = http.GET();
    // optional: print status
    Serial.print("Sheets upload HTTP code: ");
    Serial.println(code);
    http.end();
  } else {
    Serial.println("Skipping Sheets upload (no WiFi)");
  }
}

// ----------------------------------------------------
// READ CO2 (MH-Z19C) - robust, non-blocking style with retries
// ----------------------------------------------------
int readCO2() {
  // MH-Z19 command for read: 0xFF 0x01 0x86 ... 0x79
  byte cmd[9] = {0xFF,0x01,0x86,0,0,0,0,0,0x79};
  byte resp[9];
  const int maxAttempts = 2;

  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    // flush inbound to avoid stale bytes
    while (mhz19Serial.available()) mhz19Serial.read();

    mhz19Serial.write(cmd, 9);
    mhz19Serial.flush();

    unsigned long start = millis();
    int i = 0;
    // wait up to 800 ms for response (MH-Z19 can be a bit slow)
    while (millis() - start < 800) {
      if (mhz19Serial.available()) {
        int b = mhz19Serial.read();
        if (b < 0) continue;
        resp[i++] = (byte)b;
        if (i >= 9) break;
      }
    }

    if (i >= 9) {
      // checksum
      byte checksum = 0;
      for (int j = 1; j < 8; ++j) checksum += resp[j];
      checksum = 0xFF - checksum + 1;
      if (checksum == resp[8]) {
        int value = (resp[2] << 8) | resp[3];
        // sanity check on value range
        if (value >= 0 && value <= 100000) {
          return value;
        }
      } else {
        Serial.println("MHZ checksum mismatch");
      }
    } else {
      Serial.println("MHZ read incomplete");
    }
    delay(50); // brief pause before retry
  }

  // failed
  return -1;
}

// ----------------------------------------------------
// LED UPDATE
// ----------------------------------------------------
void updateLEDs(int co2){
  if(co2 <= GOOD_MAX){
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
  } 
  else if(co2 <= MODERATE_MAX){
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } 
  else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }
}

// ----------------------------------------------------
// INTRO SEQUENCE
// ----------------------------------------------------
void startupSequence(){
  Serial.println("RUNNING INTRO SEQUENCE");

  display.clearDisplay();
  centerText("Presenting", 2, 22);
  display.display();
  delay(1200);

  display.clearDisplay();
  centerText("A model", 2, 8);
  centerText("by Shourya Kaushal", 1, 36);
  display.display();
  delay(1200);

  for(int y= -20; y <= 22; y+=2){
    display.clearDisplay();
    centerText("S.V.O.E.", 3, y);
    display.display();
    delay(50);
  }
  delay(800);

  display.clearDisplay();
  centerText("Smart Ventilation", 1, 10);
  centerText("Operating", 1, 28);
  centerText("Evaluator", 1, 44);
  display.display();
  delay(1500);

  liveDisplay = true;
  lastSheetSend = millis() - sheetInterval;
}

// ----------------------------------------------------
// SAFE WiFi connect (non-blocking initial attempt with timeout)
// ----------------------------------------------------
void safeWiFiConnect() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("Attempting WiFi connect (timed)...");
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifiConnectTimeout) {
    delay(200); // short sleep, not too long
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connect timed out, continuing without WiFi");
    WiFi.disconnect(true); // ensure clean state
  }
}

// ----------------------------------------------------
// SETUP
// ----------------------------------------------------
void setup(){
  Serial.begin(115200);
  Serial.println("SETUP STARTED");

  // MH-Z19 hardware serial
  mhz19Serial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  dht.begin();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED NOT DETECTED");
    while (true) {
      delay(1000);
    }
  }
  Serial.println("OLED INITIALIZED");

  // run intro first (non-blocking)
  startupSequence();

  // try WiFi but with timeout (so intro never blocks)
  safeWiFiConnect();

  // set initial lastWifiAttempt time so reconnects happen later
  lastWifiAttempt = millis();
}

// ----------------------------------------------------
// LOOP
// ----------------------------------------------------
void loop(){
  unsigned long now = millis();

  // Periodic WiFi reconnect attempts (if disconnected)
  if (WiFi.status() != WL_CONNECTED && now - lastWifiAttempt >= wifiReconnectInterval) {
    lastWifiAttempt = now;
    Serial.println("Periodic WiFi reconnect attempt...");
    WiFi.begin(ssid, password);
    // try briefly (non-blocking)
    unsigned long tstart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - tstart < 3000) {
      delay(200);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi reconnected");
    } else {
      Serial.println("Reconnect attempt failed");
      WiFi.disconnect(true);
    }
  }

  // Read sensors
  if(now - lastSensorRead >= sensorInterval){
    lastSensorRead = now;

    int co2 = readCO2();
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // Warmup handling: discard first N readings from MH-Z19 after boot
    if (co2 > 0) {
      if (warmupCounter < WARMUP_DISCARD_READS) {
        warmupCounter++;
        Serial.print("Discarding warmup CO2 reading ");
        Serial.print(warmupCounter);
        Serial.print("/");
        Serial.println(WARMUP_DISCARD_READS);
      } else {
        co2Value = co2;
      }
    } else {
      Serial.println("CO2 read error (-1)");
    }

    if(!isnan(t)) temperature = t;
    if(!isnan(h)) humidity = h;

    updateLEDs(co2Value);
  }

  // Google Sheets
  if(liveDisplay && WiFi.status() == WL_CONNECTED && now - lastSheetSend >= sheetInterval){
    lastSheetSend = now;
    sendToGoogleSheets(co2Value, temperature, humidity);
  }

  // OLED LIVE DISPLAY (always update, even without WiFi)
  if(liveDisplay){
    display.clearDisplay();

    // CO2 text
    char co2Str[16];
    if (co2Value > 0) {
      sprintf(co2Str, "%d ppm", co2Value);
    } else {
      sprintf(co2Str, "-- ppm");
    }

    int size = (co2Value < 1000 && co2Value > 0 ? 3 : 2);
    centerText(co2Str, size, 0);

    // AIR QUALITY TEXT
    if(co2Value > 0){
      if(co2Value <= GOOD_MAX){
        centerText("GOOD", 1, 38);
      }
      else if(co2Value <= MODERATE_MAX){
        centerText("MODERATE", 1, 38);
      }
      else {
        centerText("BAD", 1, 38);
      }
    } else {
      centerText("Reading...", 1, 38);
    }

    // Temp/humidity
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.printf("T: %.1fC  H: %.1f%%", temperature, humidity);

    display.display();
  }
}
