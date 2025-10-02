# 📡 ESP32-C3 WiFi Walkie-Talkie  

A lightweight **WiFi-based walkie-talkie** built using the **ESP32-C3 Super Mini**, supporting **real-time audio transmission** over a local network. Designed for **compact, handheld use** with minimal latency and clear voice communication.  

---

## 🚀 Features
- 📶 **ESP32-C3 WiFi Communication** (no external server required)  
- 🎙️ **I2S Microphone (INMP441)** for digital audio input  
- 🔊 **I2S Audio Amplifier (MAX98357A)** with speaker output  
- 🔋 **Rechargeable LiPo Battery (LP803860)** with **TP4056 charging**  
- 💡 **LED indicators** for transmit/receive status  
- ⚡ **Low-latency, full-duplex voice streaming** over UDP  
- 🧩 **Compact design** for portable, handheld form factor  

---

## 📋 Hardware Requirements

### Components (Per Device)
- **1x ESP32-C3 Development Board** (e.g., ESP32-C3-DevKitM-1 or ESP32-C3 Super Mini for compact builds)  
- **1x I2S MEMS Microphone** (e.g., INMP441, SPH0645)  
- **1x I2S Audio Amplifier/Speaker** (e.g., MAX98357A)  
- **1x Speaker** (3W–5W recommended)  
- **1x LP803860 LiPo Battery (3.7V, ~2000mAh)** – slim, lightweight, and fits handheld enclosures  
- **1x TP4056 Charging Module** (USB-C or Micro-USB) – compact LiPo charging and protection  
- **1x LED Indicator** – shows transmission/receive status  
- **Jumper Wires**  
- **USB-C Cable** (for programming and charging/power)  

#### 🔍 Why These Components? (Form Factor Choice)
- **ESP32-C3 Super Mini** → very small footprint but powerful enough for WiFi + real-time audio.  
- **INMP441 Microphone** → digital I2S mic with built-in ADC, reducing noise and saving PCB space.  
- **MAX98357A Amplifier** → tiny I2S amp with built-in DAC, directly drives a small speaker.  
- **LP803860 Battery** → thin rectangular LiPo that provides good runtime while fitting slim enclosures.  
- **TP4056 Module** → ultra-compact LiPo charger, enabling easy USB charging without bulky circuits.  

This hardware set was chosen to keep the **walkie-talkie portable, slim, and efficient**, making it suitable for **handheld use** without compromising on audio quality.  

---

## 🛠️ Software Setup
1. Install **Arduino IDE** with **ESP32 board support**.  
2. Select **ESP32-C3 Dev Module** as the board.  
3. Install required libraries:  
   - `WiFi.h`  
   - `WiFiUdp.h`  
   - `driver/i2s.h`  
4. Flash the provided firmware (`walkie_talkie.ino`) to both ESP32-C3 devices.  

---

## ⚙️ Configuration
- Set one ESP32 as **Master (Role = 1)** and the other as **Slave (Role = 0)**.  
- Update `SSID` and `PASSWORD` in the code if using custom WiFi AP.  
- Default UDP ports:  
  - Master → `4210`  
  - Slave → `4211`  

---

## 🔋 Power Supply
- Powered via **USB-C** or **LP803860 LiPo battery**.  
- TP4056 module allows safe charging and protection.  
- Typical runtime: **6–8 hours** with 2000mAh battery.  

---

## 📦 Form Factor & Enclosure
- Designed for **handheld use** (walkie-talkie style).  
- Components selected for **slim, compact builds**.  
- Recommended: **3D-printed enclosure** with slots for ESP32, battery, speaker, and mic.  

---

## 📊 Performance
- Sample rate: **16–22 kHz**  
- Latency: **<150 ms** over local WiFi  
- Full-duplex support (can listen & talk at the same time)  

---

## 🔮 Future Improvements
- 🔒 Encrypted communication  
- 🌍 Mesh networking (ESP-NOW for no WiFi AP needed)  
- 🔋 Battery percentage monitoring via ADC  
- 🎛️ Push-to-Talk (PTT) button support  

---

## 📜 License
This project is open-source under the **MIT License**.  
