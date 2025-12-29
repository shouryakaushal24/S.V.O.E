# S.V.O.E – Smart Ventilation Operating Evaluator

## Overview
**S.V.O.E (Smart Ventilation Operating Evaluator)** is a low-cost indoor air quality monitoring system designed to measure **carbon dioxide (CO₂)** levels along with **temperature and humidity**. The system provides real-time visual feedback, alerts users when air quality becomes unsafe, and uploads data to **Google Sheets using IoT** for long-term analysis.

This project is primarily intended for **schools and classrooms**, where poor ventilation can negatively impact health, focus, and learning performance.

---

## Features
- Real-time CO₂ concentration display (ppm)
- Temperature and humidity monitoring
- LED-based air quality indication (Good / Moderate / Bad)
- OLED display for live readings and system status
- Automatic data logging to Google Sheets via WiFi
- Robust WiFi reconnection handling
- Sensor warm-up handling for accurate CO₂ readings
- Low-cost and efficient embedded system design

---

## Hardware Components
- ESP32 microcontroller (WiFi-enabled)
- MH-Z19C CO₂ sensor
- DHT22 temperature and humidity sensor
- 0.96" OLED display (SSD1306, I2C)
- Green, Yellow, and Red LEDs
- Resistors, jumper wires, and power supply

---

## LED Status Indication

| CO₂ Level (ppm) | Air Quality | LED Status |
|-----------------|------------|------------|
| ≤ 800           | Good       | Green      |
| 801 – 1200      | Moderate   | Yellow     |
| > 1200          | Bad        | Red        |

---

## OLED Display Information
The OLED screen displays:
- Current CO₂ concentration in ppm
- Air quality status (GOOD / MODERATE / BAD)
- Temperature (°C) and humidity (%)
- “Reading…” message during sensor warm-up

---

## Software & Libraries
- Arduino IDE
- Adafruit_GFX
- Adafruit_SSD1306
- DHT
- WiFi.h
- HTTPClient.h
- HardwareSerial.h

---

## System Working (High-Level)
1. On startup, the system displays an intro sequence on the OLED.
2. Sensors continuously measure CO₂, temperature, and humidity.
3. CO₂ levels are compared against predefined safety thresholds.
4. LEDs and the OLED display provide instant visual feedback.
5. Sensor data is uploaded to Google Sheets at fixed intervals when WiFi is available.
6. If WiFi disconnects, the system continues operating and automatically attempts to reconnect.

---

## Google Sheets Integration
Sensor readings are sent to Google Sheets using an HTTP GET request through a Google Apps Script endpoint.

Logged data includes:
- CO₂ concentration (ppm)
- Temperature (°C)
- Humidity (%)

This data can be used for indoor air quality analysis, system validation, and academic documentation.

---

## Configuration
The following parameters can be modified in the code:
- WiFi credentials
- CO₂ threshold values
- Sensor reading interval
- Data upload interval

```cpp
#define GOOD_MAX 800
#define MODERATE_MAX 1200
