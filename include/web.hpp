#ifndef WEB_HPP
#define WEB_HPP

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>


#define FILE_PHOTO "/photo.jpg"

namespace web {
void setup_server(AsyncWebServer* server, bool* take_photo_state, char* sta_ssid, char* sta_password);
void loop_server();
char* build_cookie(char* key, char* value);
void authenticate(AsyncWebServerRequest* request);
}

#endif