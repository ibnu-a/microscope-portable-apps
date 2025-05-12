#include <WiFi.h>
#include <WebSocketsClient.h> // Library untuk WebSockets
#include <ArduinoJson.h>      // Library untuk parsing/membuat JSON

// Ganti dengan kredensial Wi-Fi Anda
const char* ssid = "Go Green";
const char* password = "suamiku123";

// Ganti dengan alamat IP server Node.js Anda
const char* websocket_server_ip = "192.168.130.45";
const uint16_t websocket_server_port = 3000; // Port server Node.js Anda

// Konfigurasi pin untuk potensiometer
const int coarseZoomPin = 34;  // Potensiometer untuk zoom skala besar
const int fineZoomPin = 35;    // Potensiometer untuk zoom skala kecil/halus

// Objek WebSocket
WebSocketsClient webSocket;

// Variabel untuk melacak nilai potensiometer sebelumnya
int lastCoarseZoomValue = 0;
int lastFineZoomValue = 0;
const int potThreshold = 20; // Threshold perubahan nilai potensiometer

// Nilai zoom
float baseZoomLevel = 1.0;     // Zoom dasar (1x - 5x) dari potensiometer skala besar
float fineZoomFactor = 0.0;    // Faktor zoom halus (0.0 - 0.9) dari potensiometer skala kecil
float lastSentZoom = 1.0;      // Nilai zoom terakhir yang dikirim

// Fungsi handler untuk event WebSocket
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WS] Connected to url: %s\n", payload);
      break;
    case WStype_TEXT:
      Serial.printf("[WS] get text: %s\n", payload);
      // Handle any incoming messages from server if needed
      break;
    case WStype_BIN:
      Serial.printf("[WS] get binary length: %u\n", length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Koneksi Wi-Fi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Inisialisasi WebSocket
  webSocket.begin(websocket_server_ip, websocket_server_port, "/", "ws");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000); // Coba sambung ulang setiap 5 detik
  
  // Baca nilai awal potensiometer
  lastCoarseZoomValue = analogRead(coarseZoomPin);
  lastFineZoomValue = analogRead(fineZoomPin);
}

// Fungsi untuk menghitung nilai zoom dari kedua potensiometer
float calculateZoom(int coarseValue, int fineValue) {
  // Konversi nilai coarse zoom (1.0 - 5.0)
  float coarseZoom = 1.0 + (coarseValue / 4095.0) * 4.0;
  
  // Konversi nilai fine zoom (0.0 - 0.9)
  float fineZoom = (fineValue / 4095.0) * 0.9;
  
  // Gabungkan kedua nilai
  return coarseZoom + fineZoom;
}

void loop() {
  webSocket.loop(); // Menangani koneksi dan event WebSocket

  static unsigned long lastSendTime = 0;
  const unsigned long sendInterval = 50; // Poll dan kirim data setiap 50ms
  
  // Check if enough time has passed since last send
  if (millis() - lastSendTime > sendInterval) {
    // Baca nilai kedua potensiometer
    int coarseZoomValue = analogRead(coarseZoomPin);
    int fineZoomValue = analogRead(fineZoomPin);
    
    // Hitung perubahan nilai dari pembacaan terakhir
    int coarseDiff = abs(coarseZoomValue - lastCoarseZoomValue);
    int fineDiff = abs(fineZoomValue - lastFineZoomValue);
    
    // Cek apakah ada perubahan signifikan pada salah satu potensiometer
    if (coarseDiff > potThreshold || fineDiff > potThreshold) {
      // Hitung nilai zoom gabungan
      float currentZoom = calculateZoom(coarseZoomValue, fineZoomValue);
      
      // Pastikan nilai zoom tidak terlalu dekat dengan nilai sebelumnya
      if (abs(currentZoom - lastSentZoom) > 0.01) {
        // Siapkan dokumen JSON untuk dikirim
        StaticJsonDocument<128> doc;
        doc["action"] = "zoom";
        doc["level"] = currentZoom;
        
        // Kirim data jika terhubung
        if (webSocket.isConnected()) {
          String jsonMessage;
          serializeJson(doc, jsonMessage);
          webSocket.sendTXT(jsonMessage);
          Serial.printf("Sent: Zoom level = %.2f\n", currentZoom);
          
          // Simpan nilai terakhir yang dikirim
          lastSentZoom = currentZoom;
        }
      }
      
      // Update nilai terakhir
      lastCoarseZoomValue = coarseZoomValue;
      lastFineZoomValue = fineZoomValue;
    }
    
    lastSendTime = millis();
  }
}