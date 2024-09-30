#include <WiFi.h>
#include <Arduino.h>
#include <esp_now.h>

// Motor Kiri
#define pwmpin1 5
#define dir1 18
#define dir2 19

// Motor kanan
#define pwmpin2 25
#define dir3 32
#define dir4 33

#define pwmChannel1 0
#define pwmChannel2 1
#define freq 15000
#define res 8

int PWM1_DutyCycle = 0;
int maxspeed = 70;
int turnspeed = 35;
int kecepatan = 0;

const char* ssid = "ESP32-Camera-AP";
const char* password = "123456789";

char arah[8];
bool _mac = true;

// Fungsi untuk mengontrol motor dengan 4 parameter
void kontrolMotor(int d1, int d2, int d3, int d4) {
  // Tentukan target speed
  int targetSpeed;
  d1 == HIGH && d3 == HIGH ?
  targetSpeed = maxspeed :  // Jika d1 dan d3 HIGH, gunakan maxspeed
  d1 == HIGH || d2 == HIGH || d3 == HIGH || d4 == HIGH ?
  targetSpeed = turnspeed :  // Jika ada salah satu HIGH, gunakan turnspeed
  targetSpeed = 0;  // Jika semuanya LOW, kecepatan 0

  // Write pin direction
  digitalWrite(dir1, d1);
  digitalWrite(dir2, d2);
  digitalWrite(dir3, d3);
  digitalWrite(dir4, d4);

  // Tentukan apakah kecepatan naik atau turun
  bool increasing = (PWM1_DutyCycle < targetSpeed);

  if (increasing) {
    while (increasing) {
      ledcWrite(pwmChannel1, PWM1_DutyCycle++);
      ledcWrite(pwmChannel2, PWM1_DutyCycle++);
      delay(10);
    }
  } else {
    while (!increasing) {
      ledcWrite(pwmChannel1, PWM1_DutyCycle--);
      ledcWrite(pwmChannel2, PWM1_DutyCycle--);
      delay(10);
    }
  }
}

void OnDataRecv(const uint8_t* mac, const uint8_t* data, int data_len) {
  Serial.print("Data received from MAC: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);
  _mac = false;

  // Check if received data length does not exceed buffer size
  if (data_len < sizeof(arah)) {
    // Copy the received data into global variable
    memcpy(arah, data, data_len);
    arah[data_len] = '\0';  // Ensure string is null-terminated
  }

  // Print received data
  Serial.print("Arah: ");
  Serial.println(arah);

  arah == "A" ?
    kontrolMotor(LOW, LOW, HIGH, LOW):
  arah == "B" ?
    kontrolMotor(HIGH, LOW, HIGH, LOW):
  arah == "C" ?
    kontrolMotor(LOW, LOW, LOW, LOW):
  arah == "D" ?
    kontrolMotor(LOW, HIGH, LOW, HIGH):
  arah == "E" ?
    kontrolMotor(HIGH, LOW, LOW, LOW):
  delay(10);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");

  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);
  pinMode(dir3, OUTPUT);
  pinMode(dir4, OUTPUT);

  ledcSetup(pwmChannel1, freq, res);
  ledcSetup(pwmChannel2, freq, res);

  ledcAttachPin(pwmpin1, pwmChannel1);
  ledcAttachPin(pwmpin2, pwmChannel2);

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.print("Alamat MAC ESP32 B: ");
  Serial.println(WiFi.macAddress());

  // Register callback untuk menerima pesan ESP-NOW
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Tidak ada yang dilakukan di sini
}
