# ESP32-C3 WiFi Walkie-Talkie 📻

A real-time, bidirectional voice communication system using two ESP32-C3 microcontrollers with I2S audio and WiFi UDP transmission. Features low-latency audio streaming, voice activity detection, and automatic noise gating.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--C3-green.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)

## ✨ Features

- **Ultra-Low Latency**: ~4ms audio packets for near-instantaneous communication
- **Voice Activity Detection (VAD)**: Automatic transmission only when speech is detected
- **Noise Gate with Hysteresis**: Prevents audio cutting in/out during speech
- **High-Pass Filtering**: Removes DC offset and low-frequency noise (300Hz cutoff)
- **Digital Gain Control**: Adjustable microphone and speaker amplification
- **LED Indicators**: Visual feedback for transmission and reception
- **Dual Mode**: Master (Access Point) and Slave (Station) configuration
- **Network Statistics**: Monitor packets sent/received for debugging

## 📋 Hardware Requirements

### Components (Per Device)
- **1x ESP32-C3 Development Board** (e.g., ESP32-C3-DevKitM-1)
- **1x I2S MEMS Microphone** (e.g., INMP441, SPH0645)
- **1x I2S Audio Amplifier/Speaker** (e.g., MAX98357A)
- **1x Speaker** (3W-5W recommended)
- **Jumper Wires**
- **USB-C Cable** (for programming and power)

### Pin Connections

| Function | ESP32-C3 Pin | I2S Signal |
|----------|--------------|------------|
| Microphone Data | GPIO 3 | SD/DOUT |
| Speaker Data | GPIO 5 | DIN |
| Bit Clock | GPIO 7 | BCLK/SCK |
| Word Select | GPIO 6 | LRCK/WS |
| LED Indicator | GPIO 8 | - |

**Additional Connections:**
- Microphone: VDD → 3.3V, GND → GND, L/R → GND (left channel)
- Speaker Amp: VIN → 5V (or 3.3V), GND → GND
- Both devices share common ground

## 🚀 Getting Started

### Prerequisites

1. **Arduino IDE** (1.8.19+) or **PlatformIO**
2. **ESP32 Board Support**:
   - In Arduino IDE: File → Preferences → Additional Board Manager URLs
   - Add: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "esp32" by Espressif
3. **Required Libraries** (included in ESP32 core):
   - WiFi
   - WiFiUdp
   - driver/i2s

### Installation

1. Clone this repository:
```bash
git clone https://github.com/yourusername/esp32-c3-walkie-talkie.git
cd esp32-c3-walkie-talkie
```

2. Open `walkie_talkie.ino` in Arduino IDE

3. Select your board:
   - Tools → Board → ESP32 Arduino → ESP32C3 Dev Module

4. Configure the first device as **MASTER**:
```cpp
#define ROLE_MASTER 1  // Access Point
```

5. Upload to first ESP32-C3

6. Change to **SLAVE** mode:
```cpp
#define ROLE_MASTER 0  // Station
```

7. Upload to second ESP32-C3

### First Run

1. Power on the **MASTER** device first
   - LED will blink 3 times indicating ready
   - Creates WiFi network "ESP32_C3_Master"

2. Power on the **SLAVE** device
   - LED blinks while connecting
   - 3 blinks when connected successfully

3. Start talking! The LED will:
   - Stay **ON** when you're transmitting
   - **Quick blink** when receiving audio

## ⚙️ Configuration

### Audio Settings

```cpp
// Adjust these for your environment
#define AUDIO_THRESHOLD 500   // Voice detection sensitivity (lower = more sensitive)
#define HANG_TIME_MS 300      // Gate hang time in ms (prevents choppy audio)
#define MIC_GAIN 3.0f         // Microphone amplification (1.0 - 5.0)
#define SPEAKER_GAIN 2.5f     // Speaker volume (1.0 - 5.0)
```

### Network Settings

```cpp
const char* ssid = "ESP32_C3_Master";     // WiFi network name
const char* password = "12345678";         // WiFi password (min 8 chars)
#define SAMPLE_RATE 16000                  // Audio sample rate (Hz)
```

### I2S Configuration

```cpp
#define I2S_BCLK 7        // Bit clock pin
#define I2S_LRCK 6        // Left/Right clock pin
#define I2S_DIN 3         // Data in (microphone)
#define I2S_DOUT 5        // Data out (speaker)
#define PACKET_SAMPLES 64 // Samples per packet (affects latency)
```

## 🔧 Tuning Guide

### If Audio is Too Quiet
```cpp
#define MIC_GAIN 4.0f      // Increase microphone gain
#define SPEAKER_GAIN 3.5f  // Increase speaker volume
```

### If You Experience Feedback/Echo
- Increase physical distance between devices
- Reduce gains:
```cpp
#define MIC_GAIN 2.0f
#define SPEAKER_GAIN 2.0f
```

### If Voice Cuts In/Out
```cpp
#define AUDIO_THRESHOLD 300  // Lower threshold (more sensitive)
#define HANG_TIME_MS 500     // Longer hang time (less aggressive gating)
```

### If Audio Sounds Distorted
- Reduce gain values
- Check power supply (needs stable 5V/3.3V)
- Verify I2S connections

## 📊 Technical Details

### Audio Pipeline

```
Microphone → I2S DMA → High-Pass Filter → Voice Detection → 
Gate + Gain → UDP Packet → WiFi → UDP Receive → 
Gain → I2S DMA → Speaker
```

### Performance Metrics

- **Latency**: ~4ms (packet size) + ~10-30ms (network)
- **Sample Rate**: 16 kHz (telephone quality)
- **Bit Depth**: 16-bit signed PCM
- **Packet Size**: 64 samples (128 bytes)
- **Network**: UDP/IP over WiFi 802.11n
- **Range**: ~10-30 meters (depends on environment)

### Voice Activity Detection

The system uses energy-based VAD with:
1. High-pass filtering to remove DC and low-freq noise
2. Threshold comparison on filtered audio
3. Gate with hang time for natural speech

### High-Pass Filter

First-order IIR filter:
- **Cutoff**: 300 Hz
- **Purpose**: Removes breath noise, handling noise, electrical hum
- **Type**: RC high-pass

## 🐛 Troubleshooting

### Master Device Won't Start AP
- Check if WiFi credentials are valid
- Ensure no other device is using the same SSID
- Reset ESP32-C3 and try again

### Slave Can't Connect
- Verify Master is powered on first
- Check WiFi password is correct (min 8 characters)
- Ensure devices are within range (~10m initially)

### No Audio Output
- Verify I2S pin connections
- Check speaker amplifier has power
- Test with higher `SPEAKER_GAIN` value
- Monitor Serial output for "Received" packets

### No Audio Input
- Check microphone VDD is 3.3V
- Verify microphone L/R pin is grounded
- Increase `MIC_GAIN` or lower `AUDIO_THRESHOLD`
- Speak louder or closer to microphone

### Choppy/Garbled Audio
- Check WiFi signal strength
- Reduce physical obstacles between devices
- Lower `PACKET_SAMPLES` for less latency (more overhead)
- Ensure stable power supply (use USB 2.0+ ports)

### High-Pitched Feedback
- Increase distance between devices
- Reduce `MIC_GAIN` and `SPEAKER_GAIN`
- Add physical separation (different rooms)
- Cover microphone when not speaking

## 📈 Monitoring & Debugging

### Serial Monitor Output

```
=== MASTER MODE (AP) ===
AP IP: 192.168.4.1
Waiting for slave to connect...
I2S initialized
Walkie-Talkie ready!
----------------------
Sent: 1250 | Received: 1180
```

### Statistics
- View packet counts every 10 seconds
- `Sent`: Packets transmitted when speaking
- `Received`: Packets received from other device
- Difference indicates packet loss

## 🔬 Advanced Customization

### Changing Sample Rate

```cpp
#define SAMPLE_RATE 22050  // Better quality (more bandwidth)
// or
#define SAMPLE_RATE 8000   // Lower quality (less bandwidth)
```

### Adding AEC (Acoustic Echo Cancellation)

For advanced users, consider implementing:
- Adaptive filtering
- Speex DSP library
- Time-domain correlation

### Multi-Device Support

Modify UDP logic to support multiple receivers:
```cpp
// Broadcast to multiple slaves
IPAddress broadcast(192, 168, 4, 255);
udp.beginPacket(broadcast, PORT_SLAVE);
```

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 🙏 Acknowledgments

- ESP32 I2S driver by Espressif
- Inspired by classic walkie-talkie push-to-talk systems
- Thanks to the Arduino and ESP32 communities

## 📧 Contact

Your Name - [@yourtwitter](https://twitter.com/yourtwitter)

Project Link: [https://github.com/yourusername/esp32-c3-walkie-talkie](https://github.com/yourusername/esp32-c3-walkie-talkie)

---

**⭐ If you found this project helpful, please consider giving it a star!**