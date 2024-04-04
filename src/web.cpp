#include "web.hpp"

namespace web {
const char* DEFAULT_USERNAME = "user";
const char* DEFAULT_PASSWORD = "user";

void setup_server(AsyncWebServer* server, bool* take_photo_state) {
  server->serveStatic("/", SPIFFS, "/")
      .setDefaultFile("index.html")
      .setAuthentication("user", "pass");
  ;

  // why I'm using GET for things that are supposed to use POST:
  // html forms POST by default uses body json
  // adding json support would mean more code for the esp
  // custom html form implementation also adds up length to the html file

  server->on("/api/wifi_status", HTTP_GET, [](AsyncWebServerRequest* request) {
    char status[4];
    switch (WiFi.status()) {
      case WL_NO_SHIELD:
        strcpy(status, "255");
        break;
      case WL_IDLE_STATUS:
        strcpy(status, "0");
        break;
      case WL_NO_SSID_AVAIL:
        strcpy(status, "1");
        break;
      case WL_SCAN_COMPLETED:
        strcpy(status, "2");
        break;
      case WL_CONNECTED:
        strcpy(status, "3");
        break;
      case WL_CONNECT_FAILED:
        strcpy(status, "4");
        break;
      case WL_CONNECTION_LOST:
        strcpy(status, "5");
        break;
      case WL_DISCONNECTED:
        strcpy(status, "6");
        break;
    }
    request->send_P(200, "text/plain", status);
  });

  server->begin();
}

void wifi_get(AsyncWebServer* server, char* sta_ssid, char* sta_password) {
  server->on("/api/wifi", HTTP_GET,
             [sta_ssid, sta_password](AsyncWebServerRequest* request) {
               char json[512];
               snprintf(json, 512 * sizeof(char),
                        "{\"ssid\":\"%s\",\"pw\":\"%s\"}", sta_ssid,
                        sta_password);

               AsyncWebServerResponse* response =
                   request->beginResponse(200, "text/json", json);
               request->send(response);
             });
}

void wifi_post(AsyncWebServer* server, char* sta_ssid,
               unsigned int sta_ssid_len, char* sta_password,
               unsigned int sta_password_len) {
  server->on("/api/wifi", HTTP_POST,
             [sta_ssid, sta_ssid_len, sta_password,
              sta_password_len](AsyncWebServerRequest* request) {
               if (!(request->hasParam("ssid") && request->hasParam("pw")))
                 return request->send_P(
                     400, "text/plain",
                     "Bad request: requires ssid and pw parameter");

               String ssid = request->getParam("ssid")->value();
               String password = request->getParam("pw")->value();

               Serial.println(ssid);
               Serial.println(password);

               if (ssid.length() > sta_ssid_len ||
                   password.length() > sta_password_len)
                 return request->send_P(
                     400, "text/plain",
                     "Bad request: ssid and parameter length overflow");

               ssid.toCharArray(sta_ssid, sta_ssid_len);
               password.toCharArray(sta_password, sta_password_len);

               WiFi.disconnect();
               WiFi.begin(sta_ssid, sta_password);

               return request->send_P(200, "text/plain", "Success");
             });
}

}  // namespace web