#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Wi-Fi Configuration
const char* ssid = "Den den mushi";
const char* password = "1sampai9";

// Backend Server Configuration
const char* backendHost = "192.168.53.45"; // Ganti dengan IP Address mesin yang menjalankan backend Node.js Anda
const int backendPort = 3000;
const char* sensorDataPath = "/sensor-data";
const char* imageActionPath = "/image-action";

const char* API_KEY = "VE4D209rhTcgH1KNxj4YdUr0mj9eZFRgnU3DiQPjnzWAfKGTVYCAqNOj4sjfSxTqnYlPAmofSZ9VJDVG4ag0vC22xAfWSzau3sWNrjCk1zd23tiJUx0OXOMMSqgyPgeB";

// Hardware Pins
const int zoomPotPin = 34; // Potensiometer untuk pergeseran zoom-in zoom-out
const int panPotPin = 35; // Potensiometer untuk pergeseran kanan-kiri
const int tiltPotPin = 32; // Potensiometer untuk pergeseran atas-bawah
const int ledPin = 2;

// Rotary Encoder Pins
const int encoderCLK = 16; // CLK pin dari Rotary Encoder rx
const int encoderDT = 17;  // DT pin dari Rotary Encoder tx
const int encoderSW = 18;  // SW pin (button) dari Rotary Encoder

// Control Variables (tidak terlalu digunakan lagi, diganti currentZoomLevel/PanPosition/TiltPosition)
// float zoomLevel = 1.0;
// float panPosition = 0.0;
// float tiltPosition = 0.0;

// Smoothing Configuration (numReadings tidak relevan untuk LPF)
const int numReadings = 2;
int zoomReadings[numReadings];
int panReadings[numReadings];
int tiltReadings[numReadings];
int zoomReadIndex = 0;
int panReadIndex = 0;
int tiltReadIndex = 0;
long zoomTotal = 0;
long panTotal = 0;
long tiltTotal = 0;

// Control Thresholds
int lastZoomValue = 0; // lastSmoothedValue untuk LPF
int lastPanValue = 0;  // lastSmoothedValue untuk LPF
int lastTiltValue = 0; // lastSmoothedValue untuk LPF Tilt

// Timing Control
unsigned long lastZoomSentTime = 0;
unsigned long lastPanSentTime = 0;
unsigned long lastTiltSentTime = 0;
const unsigned long minSendInterval = 20;

// Rotary Encoder State
volatile int encoderPos = 0;
volatile unsigned long lastEncoderChangeTime = 0;
int lastEncoderCLKState;
const unsigned long encoderDebounceDelay = 10;
unsigned long lastEncoderButtonPressTime = 0;
const unsigned long encoderButtonDebounceDelay = 50;
const unsigned long minEncoderSendInterval = 300;

// Connection Status
bool wifiConnected = false;
unsigned long lastConnectionCheck = 0;
const unsigned long connectionCheckInterval = 5000;

// Microscope Constants
const float MIN_ZOOM = 0.5f;
const float MAX_ZOOM = 5.0f;
const float MIN_PAN = -100.0f;
const float MAX_PAN = 100.0f;
const float MIN_TILT = -100.0f;
const float MAX_TILT = 100.0f;

// Variable for ADC smoothing/filtering selection
enum FilterType { MOVING_AVERAGE, LOW_PASS };
FilterType currentFilter = LOW_PASS;

// Low-Pass Filter Coefficients 
// TUNING SENSITIVITY
float alphaZoom = 0.6;
float alphaPan = 0.6;
float alphaTilt = 0.6;

// Global variables to hold the current state
float currentZoomLevel = 1.0;
float currentPanPosition = 0.0;
float currentTiltPosition = 0.0;
float currentZoomPercentage = 0.0;
float currentPanPercentage = 0.0;
float currentTiltPercentage = 0.0;

// Forward declarations
void sendMicroscopeData(float zoom, float pan, float tilt, float zoomPerc, float panPerc, float tiltPerc);
void IRAM_ATTR readEncoder();
void sendImageNavigationAction(const char* action); // Pastikan ini dideklarasikan

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Digital Microscope Controller ===");

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(zoomPotPin, INPUT);
  pinMode(panPotPin, INPUT);
  pinMode(tiltPotPin, INPUT);

  pinMode(encoderCLK, INPUT_PULLUP);
  pinMode(encoderDT, INPUT_PULLUP);
  pinMode(encoderSW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderCLK), readEncoder, CHANGE);
  lastEncoderCLKState = digitalRead(encoderCLK);

  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  initializeSmoothingArrays();
  setupWiFi();
  initializePotentiometers();

  Serial.println("System ready!");
  printControlInstructions();

  if (wifiConnected) {
    sendMicroscopeData(currentZoomLevel, currentPanPosition, currentTiltPosition,
                       currentZoomPercentage, currentPanPercentage, currentTiltPercentage);
  }
}

void initializeSmoothingArrays() {
  for (int i = 0; i < numReadings; i++) {
    zoomReadings[i] = 0;
    panReadings[i] = 0;
    tiltReadings[i] = 0;
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    Serial.print(".");
    blinkLED(100);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi Connected!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("\nWiFi Connection Failed!");
    wifiConnected = false;
  }
}

void initializePotentiometers() {
  int initialZoom = analogRead(zoomPotPin);
  int initialPan = analogRead(panPotPin);
  int initialTilt = analogRead(tiltPotPin);

  lastZoomValue = initialZoom;
  lastPanValue = initialPan;
  lastTiltValue = initialTilt;

  for (int i = 0; i < numReadings; i++) {
    zoomReadings[i] = initialZoom;
    panReadings[i] = initialPan;
    tiltReadings[i] = initialTilt;
  }

  currentZoomLevel = mapToZoom(lastZoomValue);
  currentPanPosition = mapToPan(lastPanValue);
  currentTiltPosition = mapToTilt(lastTiltValue);
  currentZoomPercentage = ((currentZoomLevel - MIN_ZOOM) / (MAX_ZOOM - MIN_ZOOM)) * 100;
  currentPanPercentage = ((currentPanPosition + 100) / 200) * 100;
  currentTiltPercentage = ((currentTiltPosition + 100) / 200) * 100;

  Serial.printf("Initial values - Zoom: %.2fx, Pan: %.1f, Tilt: %.1f\n", currentZoomLevel, currentPanPosition, currentTiltPosition);
}

float mapToZoom(int raw) {
  float normalized = constrain(raw / 4095.0f, 0.0f, 1.0f);
  return MIN_ZOOM + (normalized * (MAX_ZOOM - MIN_ZOOM));
}

float mapToPan(int raw) {
  float normalized = constrain(raw / 4095.0f, 0.0f, 1.0f);
  float pan = (normalized - 0.5f) * 200.0f;
  // TUNING SENSITIVITY
  if (abs(pan) < 2.0f) {
    pan = 0.0f;
  }
  return constrain(pan, MIN_PAN, MAX_PAN);
}

float mapToTilt(int raw) {
  float normalized = constrain(raw / 4095.0f, 0.0f, 1.0f);
  float tilt = (normalized - 0.5f) * 200.0f;
  // TUNING SENSITIVITY
  if (abs(tilt) < 2.0f) {
    tilt = 0.0f;
  }
  return constrain(tilt, MIN_TILT, MAX_TILT);
}

int smoothAnalogReadMovingAverage(int pin, int* readings, int& readIndex, long& total) {
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(pin);
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;
  return total / numReadings;
}

int smoothAnalogReadLowPass(int pin, int& lastSmoothedValue, float alpha) {
  int rawValue = analogRead(pin);
  lastSmoothedValue = alpha * rawValue + (1 - alpha) * lastSmoothedValue;
  return lastSmoothedValue;
}

void sendMicroscopeData(float zoom, float pan, float tilt, float zoomPerc, float panPerc, float tiltPerc) {
  if (!wifiConnected) {
    checkConnections();
    return;
  }

  StaticJsonDocument<350> doc; // Ditingkatkan ukurannya

  doc["zoom_level"] = round(zoom * 1000) / 1000.0;
  doc["pan_position"] = round(pan * 100) / 100.0;
  doc["tilt_position"] = round(tilt * 100) / 100.0;

  doc["zoom_percentage"] = round(zoomPerc * 100) / 100.0;
  doc["pan_percentage"] = round(panPerc * 100) / 100.0;
  doc["tilt_percentage"] = round(tiltPerc * 100) / 100.0;

  String jsonPayload;
  if (serializeJson(doc, jsonPayload) == 0) {
      Serial.println("✗ Failed to serialize JSON for microscope data!");
      errorBlink();
      return;
  }

  HTTPClient http;
  String serverUrl = "http://" + String(backendHost) + ":" + String(backendPort) + String(sensorDataPath);

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("✓ Pot. Data sent. HTTP %d. Payload: %s\n", httpResponseCode, jsonPayload.c_str());
    quickBlink();
  } else {
    Serial.printf("✗ Pot. Data Error: %s (Code: %d)\n", http.errorToString(httpResponseCode).c_str(), httpResponseCode);
    errorBlink();
  }
  http.end();
}

void sendImageNavigationAction(const char* action) {
  if (!wifiConnected) {
    checkConnections();
    return;
  }

  StaticJsonDocument<64> doc;
  doc["action"] = action;

  String jsonPayload;
  if (serializeJson(doc, jsonPayload) == 0) {
      Serial.println("✗ Failed to serialize JSON for image action!");
      errorBlink();
      return;
  }

  HTTPClient http;
  String serverUrl = "http://" + String(backendHost) + ":" + String(backendPort) + String(imageActionPath);

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("✓ Image Action sent: %s. HTTP %d\n", action, httpResponseCode);
    quickBlink();
  } else {
    Serial.printf("✗ Image Action Error: %s (Code: %d)\n", http.errorToString(httpResponseCode).c_str(), httpResponseCode);
    errorBlink();
  }
  http.end();
}

void checkZoomPotentiometer() {
  int currentVal;
  if (currentFilter == MOVING_AVERAGE) {
    currentVal = smoothAnalogReadMovingAverage(zoomPotPin, zoomReadings, zoomReadIndex, zoomTotal);
  } else {
    currentVal = smoothAnalogReadLowPass(zoomPotPin, lastZoomValue, alphaZoom);
  }

  float newZoom = mapToZoom(currentVal);

  // TUNING SENSITIVITY
  if (abs(newZoom - currentZoomLevel) > 0.01f) {
    currentZoomLevel = newZoom;
    currentZoomPercentage = ((currentZoomLevel - MIN_ZOOM) / (MAX_ZOOM - MIN_ZOOM)) * 100;
    Serial.printf("Zoom Updated: Raw=%d, Smoothed=%d, Mapped=%.2fx (%.0f%%)\n",
                  analogRead(zoomPotPin), currentVal, currentZoomLevel, currentZoomPercentage);

    unsigned long currentTime = millis();
    if (currentTime - lastZoomSentTime >= minSendInterval) {
      sendMicroscopeData(currentZoomLevel, currentPanPosition, currentTiltPosition,
                         currentZoomPercentage, currentPanPercentage, currentTiltPercentage);
      lastZoomSentTime = currentTime;
    }
  }
}

void checkPanPotentiometer() {
  int currentVal;
  if (currentFilter == MOVING_AVERAGE) {
    currentVal = smoothAnalogReadMovingAverage(panPotPin, panReadings, panReadIndex, panTotal);
  } else {
    currentVal = smoothAnalogReadLowPass(panPotPin, lastPanValue, alphaPan);
  }

  float newPan = mapToPan(currentVal);

  if (abs(newPan - currentPanPosition) > 0.5f) {
    currentPanPosition = newPan;
    currentPanPercentage = ((currentPanPosition + 100) / 200) * 100;
    Serial.printf("Pan Updated: Raw=%d, Smoothed=%d, Mapped=%.1f (%.0f%%)\n",
                  analogRead(panPotPin), currentVal, currentPanPosition, currentPanPercentage);

    unsigned long currentTime = millis();
    if (currentTime - lastPanSentTime >= minSendInterval) {
      sendMicroscopeData(currentZoomLevel, currentPanPosition, currentTiltPosition,
                         currentZoomPercentage, currentPanPercentage, currentTiltPercentage);
      lastPanSentTime = currentTime;
    }
  }
}

void checkTiltPotentiometer() {
  int currentVal;
  if (currentFilter == MOVING_AVERAGE) {
    currentVal = smoothAnalogReadMovingAverage(tiltPotPin, tiltReadings, tiltReadIndex, tiltTotal);
  } else {
    currentVal = smoothAnalogReadLowPass(tiltPotPin, lastTiltValue, alphaTilt);
  }

  float newTilt = mapToTilt(currentVal);

  if (abs(newTilt - currentTiltPosition) > 0.5f) {
    currentTiltPosition = newTilt;
    currentTiltPercentage = ((currentTiltPosition + 100) / 200) * 100;
    Serial.printf("Tilt Updated: Raw=%d, Smoothed=%d, Mapped=%.1f (%.0f%%)\n",
                  analogRead(tiltPotPin), currentVal, currentTiltPosition, currentTiltPercentage);

    unsigned long currentTime = millis();
    if (currentTime - lastTiltSentTime >= minSendInterval) {
      sendMicroscopeData(currentZoomLevel, currentPanPosition, currentTiltPosition,
                         currentZoomPercentage, currentPanPercentage, currentTiltPercentage);
      lastTiltSentTime = currentTime;
    }
  }
}

// ISR untuk Rotary Encoder (Dijalankan saat CLK berubah)
void IRAM_ATTR readEncoder() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderChangeTime < encoderDebounceDelay) {
    return;
  }

  int newCLKState = digitalRead(encoderCLK);
  if (newCLKState != lastEncoderCLKState) {
    lastEncoderCLKState = newCLKState;

    if (digitalRead(encoderDT) != newCLKState) { // Clockwise
      encoderPos++;
    } else { // Counter-Clockwise
      encoderPos--;
    }
    lastEncoderChangeTime = currentTime;
  }
}

// Memeriksa dan memproses Rotary Encoder
void checkEncoder() {
  unsigned long currentTime = millis();

  // Memproses rotasi encoder
  if (encoderPos != 0) {
    if (currentTime - lastEncoderChangeTime >= minEncoderSendInterval) { // Cooldown untuk kirim
      if (encoderPos > 0) {
        sendImageNavigationAction("next");
      } else {
        sendImageNavigationAction("previous");
      }
      encoderPos = 0; // Reset posisi encoder setelah aksi dikirim
      // lastEncoderChangeTime = currentTime; // Sudah di update di ISR
    }
  }

  // Memproses tombol encoder (jika ada)
  if (digitalRead(encoderSW) == LOW) { // Tombol ditekan (INPUT_PULLUP)
    if (currentTime - lastEncoderButtonPressTime > encoderButtonDebounceDelay) { // Menggunakan variabel baru
      Serial.println("Encoder Button Pressed!");
      // sendImageNavigationAction("select"); // Contoh: Jika Anda ingin aksi tombol encoder
      lastEncoderButtonPressTime = currentTime; // Update waktu terakhir tombol encoder ditekan
    }
  }
}

void checkConnections() {
  unsigned long currentTime = millis();

  if (currentTime - lastConnectionCheck > connectionCheckInterval) {
    lastConnectionCheck = currentTime;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      wifiConnected = false;
      WiFi.reconnect();
    } else {
      wifiConnected = true;
    }
  }
}

// Utility functions
void blinkLED(int duration) {
  digitalWrite(ledPin, HIGH);
  delay(duration);
  digitalWrite(ledPin, LOW);
  delay(duration);
}

void quickBlink() {
  digitalWrite(ledPin, LOW);
  delay(10);
  digitalWrite(ledPin, HIGH);
}

void errorBlink() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }
}

void printControlInstructions() {
  Serial.println("\n=== Digital Microscope Controller ===");
  Serial.println("Zoom Potentiometer: 0.5x - 5.0x zoom");
  Serial.println("Pan Potentiometer: -100 to +100 horizontal pan");
  Serial.println("Tilt Potentiometer: -100 to +100 vertical pan");
  Serial.println("Rotary Encoder: Rotate for image navigation (next/previous), Push for select (optional)");
  Serial.println("LED: Steady = Connected, Blink = Activity, Fast blink = Error");
  Serial.println("============================\n");
}

void loop() {
  checkZoomPotentiometer();
  checkPanPotentiometer();
  checkTiltPotentiometer();
  checkEncoder();
  checkConnections();
  delay(5);
}