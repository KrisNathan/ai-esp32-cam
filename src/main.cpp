#include <WiFi.h>
#include <esp_camera.h>
#include <esp_timer.h>
#include <img_converters.h>
#include <Arduino.h>
#include <soc/soc.h>           // Disable brownour problems
#include <soc/rtc_cntl_reg.h>  // Disable brownour problems
#include <driver/rtc_io.h>
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>
#include "cam.hpp"
#include "web.hpp"
#include "push_img.hpp"

#define STA_SSID_LEN 64
#define STA_PASSWORD_LEN 128

char sta_ssid[STA_SSID_LEN] = "SSID"; // 32 char * 2
char sta_password[STA_PASSWORD_LEN] = "PASSWORD"; // 64 char * 2

const char* AP_SSID = "aicam";
const char* AP_PASSWORD = "1234567890";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
boolean takeNewPhoto = false;

void setup() {
  delay(5000);
  Serial.begin(115200); // Serial port for debugging purposes

  WiFi.mode(WIFI_AP_STA);

  if (!WiFi.softAP(AP_SSID, AP_PASSWORD))
    Serial.println("Failed setting up WiFi AP.");
  Serial.printf("WiFi AP Address: %s\n", WiFi.softAPIP().toString());

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
      Serial.print("FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
  }

  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP().toString());

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Camera init
  esp_err_t err = setup_cam();
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
  
  web::wifi_get(&server, sta_ssid, sta_password);
  web::wifi_post(&server, sta_ssid, STA_SSID_LEN, sta_password, STA_PASSWORD_LEN);
  web::setup_server(&server, &takeNewPhoto);
}

wl_status_t previousWiFiStatus = WL_IDLE_STATUS;
wl_status_t currentWiFiStatus = WL_IDLE_STATUS;

void loop() {
  currentWiFiStatus = WiFi.status();
  if (currentWiFiStatus != previousWiFiStatus) {
    if (currentWiFiStatus == WL_CONNECTED) {
      Serial.printf("IP Address: http://%s\n", WiFi.localIP().toString().c_str());
    }
    previousWiFiStatus = currentWiFiStatus;
  }

  if (currentWiFiStatus == WL_CONNECTED) {
    push_img(capture_photo_base64());
  }
  delay(500);
}
