#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "esp_camera.h"

// Konfigurasi WiFi Access Point
const char* ssid = "ESP-Access-Point";
const char* password = "123456789";

// Konfigurasi untuk koneksi ke ESP32 embedded (motor)
IPAddress motorIP(192, 168, 4, 2);  // Sesuaikan dengan IP dari ESP32 embedded
WiFiClient motorClient;
const int motorPort = 1234;  // Port untuk komunikasi ke ESP32 embedded

WiFiServer tcpServer(80);  // Server TCP untuk menerima string dari komputer
WiFiServer rtspServer(554); // Server RTSP

void startCameraServer();

// Konfigurasi pin kamera OV2640
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM      5
#define Y2_GPIO_NUM      4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi kamera OV2640
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA; // Resolusi 1600x1200
    config.jpeg_quality = 10; // Kualitas JPEG
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA; // Resolusi 800x600
    config.jpeg_quality = 12; // Kualitas JPEG
    config.fb_count = 1;
  }

  // Inisialisasi kamera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera gagal diinisialisasi: 0x%x", err);
    return;
  }

  // Setup Access Point WiFi
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  // Mulai server TCP dan RTSP
  tcpServer.begin();
  rtspServer.begin();
  Serial.println("RTSP dan TCP server telah dimulai.");

  startCameraServer();
}

void loop() {
  // Menerima koneksi TCP dari komputer untuk menerima string
  WiFiClient tcpClient = tcpServer.available();
  if (tcpClient) {
    if (tcpClient.connected()) {
      String receivedString = tcpClient.readStringUntil('\n');  // Baca string dari komputer
      Serial.print("String diterima dari PC: ");
      Serial.println(receivedString);

      // Mengirimkan string ke ESP32 embedded (motor)
      if (motorClient.connect(motorIP, motorPort)) {
        motorClient.print(receivedString);
        Serial.print("String diteruskan ke motor: ");
        Serial.println(receivedString);
        motorClient.stop();  // Tutup koneksi setelah mengirimkan string
      }
    }
  }
}

// Placeholder fungsi untuk memulai server kamera RTSP
void startCameraServer() {
  // Implementasi untuk menangani stream RTSP dari kamera OV2640
  Serial.println("Starting RTSP stream...");
}