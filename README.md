# S.V.O.E – Smart Ventilation Operating Evaluator 🌬️

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

| CO₂ Level (ppm) | Air Quality | LED Color |
|-----------------|------------|-----------|
| ≤ 800           | Good       | Green     |
| 801 – 1200      | Moderate   | Yellow    |
| > 1200          | Bad        | Red       |

---

## OLED Display Information
The OLED screen displays:
- Current CO₂ concentration (ppm)
- Air quality status (GOOD / MODERATE / BAD)
- Temperature (°C) and humidity (%)
- `Reading...` message during sensor warm-up

---

## Software & Libraries
- Arduino IDE
- `Adafruit_GFX`
- `Adafruit_SSD1306`
- `DHT`
- `WiFi.h`
- `HTTPClient.h`
- `HardwareSerial.h`

---

## System Workflow (High-Level)
1. On startup, the system displays an intro sequence on the OLED.
2. Sensors continuously measure CO₂, temperature, and humidity.
3. CO₂ levels are evaluated against predefined safety thresholds.
4. LEDs and the OLED display provide instant visual feedback.
5. Sensor data is uploaded to Google Sheets at fixed intervals when WiFi is available.
6. If WiFi disconnects, the system continues operating locally and automatically attempts to reconnect.

---

## Google Sheets Integration
Sensor readings are sent to Google Sheets using an **HTTP GET request** via a **Google Apps Script endpoint**.

Logged data includes:
- CO₂ concentration (ppm)
- Temperature (°C)
- Humidity (%)

This data can be used for:
- Indoor air quality analysis
- Long-term trend monitoring
- Academic reports and validation

---

## Configuration
The following parameters can be modified directly in the code:
- WiFi credentials
- CO₂ threshold values
- Sensor reading interval
- Data upload interval

```cpp
#define GOOD_MAX 800
#define MODERATE_MAX 1200

---

## Reliability Features
- Non-blocking sensor reading logic for smooth operation
- MH-Z19C CO₂ sensor warm-up discard handling for stable readings
- Checksum validation to ensure accurate CO₂ measurements
- Safe WiFi connection with timeout protection
- Automatic WiFi reconnection attempts
- System continues functioning locally even without WiFi

---

## Setup & Usage
1. Connect all hardware components according to the wiring configuration.
2. Update WiFi credentials and the Google Sheets script URL in the code.
3. Upload the code to the ESP32 using Arduino IDE.
4. Power the system and allow the CO₂ sensor to warm up.
5. Monitor readings on the OLED display and LEDs.
6. View logged data in the connected Google Sheets document.

---

## Applications
- Classrooms and schools
- Offices and meeting rooms
- Libraries
- Laboratories
- Any enclosed indoor environment requiring ventilation monitoring

---

## Author
**Shourya Kaushal**  
*S.V.O.E – Smart Ventilation Operating Evaluator*  
Embedded Systems & IoT Project

---

## License
This project is intended for **educational and academic use**.  
