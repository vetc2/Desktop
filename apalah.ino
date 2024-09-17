#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include "esp_http_server.h"

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "ESP-cam";
const char *password = "123456789";

// =====================
// RTSP Server Settings
// =====================
static const char *RTSP_URL = "/stream";
static const char *RTSP_CONTENT_TYPE = "application/sdp";

void startCameraServer();
void setupLedFlash(int pin);

// RTSP server task
void rtspTask(void *pvParameters) {
  httpd_handle_t server = (httpd_handle_t) pvParameters;

  // RTSP session handler
  httpd_uri_t uri_rtsp = {
    .uri = RTSP_URL,
    .method = HTTP_GET,
    .handler = [](httpd_req_t *req) -> esp_err_t {
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }

      httpd_resp_set_type(req, RTSP_CONTENT_TYPE);
      httpd_resp_send(req, (const char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
      return ESP_OK;
    },
    .user_ctx = NULL
  };

  httpd_register_uri_handler(server, &uri_rtsp);

  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Camera initialization
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
  config.pixel_format = PIXFORMAT_JPEG; // For streaming
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Set up WiFi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Create the RTSP server
  httpd_handle_t server = NULL;
  httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
  config_http.server_port = 80;

  if (httpd_start(&server, &config_http) == ESP_OK) {
    xTaskCreatePinnedToCore(rtspTask, "rtspTask", 4096, server, 1, NULL, 0);
  }

  Serial.print("Camera Ready! Use 'rtsp://");
  Serial.print(IP);
  Serial.println(RTSP_URL);
}

void loop() {
  delay(10000); // Keep the server alive
}
