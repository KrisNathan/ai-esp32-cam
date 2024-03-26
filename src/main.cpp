// #include <Arduino.h>

// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   Serial.println("Hello!!!");
//   delay(1000);
// }

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/
  
  IMPORTANT!!! 
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

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

// Replace with your network credentials
const char* STA_SSID = "TPI NETWORK";
const char* STA_PASSWORD = "privater362000";

const char* AP_SSID = "aecam";
const char* AP_PASSWORD = "1234567890";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;

void setup() {
  delay(5000);
  // Serial port for debugging purposes
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);

  if (!WiFi.softAP(AP_SSID, AP_PASSWORD))
    Serial.println("Failed setting up WiFi AP.");
  Serial.printf("WiFi AP Address: %s\n", WiFi.softAPIP().toString());

  WiFi.begin(STA_SSID, STA_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting to WiFi...: ");
    Serial.println(WiFi.status());
  }

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
  
  web::setup_server(&server, &takeNewPhoto);
}

void loop() {
  if (takeNewPhoto) {
    capture_photo_save_spiffs();
    takeNewPhoto = false;
  }
  delay(1);
}
