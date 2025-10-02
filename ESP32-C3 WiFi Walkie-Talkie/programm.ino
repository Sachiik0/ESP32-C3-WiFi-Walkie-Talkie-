#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "driver/i2s.h"

// ==========================
// CONFIG
// ==========================

// Select role: MASTER = AP, SLAVE = STA
#define ROLE_MASTER 1 // set 1 for master, 0 for slave

// Wi-Fi
const char* ssid = "ESP32_C3_Master";
const char* password = "12345678";
WiFiUDP udp;
const int PORT_MASTER = 4210;
const int PORT_SLAVE = 4211;
IPAddress masterIP(192, 168, 4, 1);
IPAddress slaveIP(192, 168, 4, 2);

// I2S Config
#define I2S_BCLK 7
#define I2S_LRCK 6
#define I2S_DIN 3   // Microphone
#define I2S_DOUT 5  // Speaker
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define DMA_BUF_LEN 64
#define DMA_BUF_COUNT 4
#define PACKET_SAMPLES 64   // Low latency!

// LED
#define LED_PIN 8
#define AUDIO_THRESHOLD 500
#define HANG_TIME_MS 300

// Gains - tuned to prevent feedback while staying loud
#define MIC_GAIN 3.0f      // Reduced from 4.0
#define SPEAKER_GAIN 2.5f  // Reduced from 4.0

// Buffers
int16_t i2s_read_buf[PACKET_SAMPLES];
int16_t pcm_buf[PACKET_SAMPLES];
int16_t audio_buf[PACKET_SAMPLES];

// State
bool gateOpen = false;
unsigned long lastVoiceTime = 0;
bool ledState = false;
unsigned long lastLedToggle = 0;

// High-pass filter state
float hp_lastInput = 0, hp_lastOutput = 0;

// Stats
unsigned long packetsSent = 0;
unsigned long packetsReceived = 0;
unsigned long lastStatsTime = 0;

// ==========================
// HELPERS
// ==========================
int16_t highPassFilter(int16_t input) {
  const float RC = 1.0 / (2.0 * 3.14159265359 * 300.0);
  const float dt = 1.0 / SAMPLE_RATE;
  const float alpha = RC / (RC + dt);
  
  float output = alpha * (hp_lastOutput + (float)input - hp_lastInput);
  hp_lastInput = (float)input;
  hp_lastOutput = output;
  
  // Clamp to prevent overflow
  if (output > 32767.0f) output = 32767.0f;
  if (output < -32768.0f) output = -32768.0f;
  
  return (int16_t)output;
}

void applyDigitalGain(int16_t *buf, int samples, float gain) {
  if (gain == 1.0f) return;
  for (int i = 0; i < samples; i++) {
    int32_t v = (int32_t)buf[i] * (int32_t)(gain * 256.0f);
    v >>= 8;
    if (v > 32767) v = 32767;
    else if (v < -32768) v = -32768;
    buf[i] = (int16_t)v;
  }
}

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRCK,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_DIN
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

void printStats() {
  unsigned long now = millis();
  if (now - lastStatsTime > 10000) { // Every 10 seconds
    Serial.print("Sent: ");
    Serial.print(packetsSent);
    Serial.print(" | Received: ");
    Serial.println(packetsReceived);
    lastStatsTime = now;
  }
}

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

#if ROLE_MASTER
  Serial.println("=== MASTER MODE (AP) ===");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(myIP);
  udp.begin(PORT_MASTER);
  Serial.println("Waiting for slave to connect...");
#else
  Serial.println("=== SLAVE MODE (STA) ===");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to AP");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    Serial.print(".");
    delay(200);
  }
  digitalWrite(LED_PIN, LOW);
  Serial.println("\nConnected!");
  Serial.print("STA IP: ");
  Serial.println(WiFi.localIP());
  udp.begin(PORT_SLAVE);
#endif

  setupI2S();
  Serial.println("I2S initialized");
  Serial.println("Walkie-Talkie ready!");
  Serial.println("----------------------");
  
  // Startup indicator - 3 quick blinks
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

// ==========================
// LOOP
// ==========================
void loop() {
  // === Transmit mic audio ===
  size_t bytes_read = 0;
  if (i2s_read(I2S_PORT, (void*)i2s_read_buf, sizeof(i2s_read_buf), &bytes_read, portMAX_DELAY) == ESP_OK) {
    int samples = bytes_read / sizeof(int16_t);
    bool voiceDetected = false;
    
    // Process audio: high-pass filter + voice detection
    for (int i = 0; i < samples; i++) {
      int16_t pcm = highPassFilter(i2s_read_buf[i]);
      pcm_buf[i] = pcm;
      if (abs(pcm) > AUDIO_THRESHOLD) {
        voiceDetected = true;
      }
    }
    
    // Gate logic with hysteresis
    if (voiceDetected) {
      gateOpen = true;
      lastVoiceTime = millis();
      digitalWrite(LED_PIN, HIGH);
    } else if (gateOpen && (millis() - lastVoiceTime > HANG_TIME_MS)) {
      gateOpen = false;
      digitalWrite(LED_PIN, LOW);
    }
    
    // Transmit when gate is open
    if (gateOpen) {
      applyDigitalGain(pcm_buf, samples, MIC_GAIN);
#if ROLE_MASTER
      udp.beginPacket(slaveIP, PORT_SLAVE);
#else
      udp.beginPacket(masterIP, PORT_MASTER);
#endif
      udp.write((uint8_t*)pcm_buf, samples * sizeof(int16_t));
      udp.endPacket();
      packetsSent++;
    }
  }

  // === Receive speaker audio ===
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    int len = udp.read((uint8_t*)audio_buf, sizeof(audio_buf));
    if (len > 0) {
      int samples = len / sizeof(int16_t);
      applyDigitalGain(audio_buf, samples, SPEAKER_GAIN);
      
      size_t bytes_written;
      i2s_write(I2S_PORT, audio_buf, len, &bytes_written, portMAX_DELAY);
      packetsReceived++;
      
      // Quick blink to indicate received audio (only if not transmitting)
      if (!gateOpen) {
        digitalWrite(LED_PIN, HIGH);
        delayMicroseconds(500); // Very short blink
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
  
  // Print stats occasionally
  printStats();
}