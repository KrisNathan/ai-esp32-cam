#ifndef WEB_HPP
#define WEB_HPP

#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>

#define FILE_PHOTO "/photo.jpg"

namespace web {
void setup_server(AsyncWebServer* server, bool* take_photo_state);
void wifi_get(AsyncWebServer* server, char* sta_ssid, char* sta_password);
void wifi_post(AsyncWebServer* server, char* sta_ssid,
               unsigned int sta_ssid_len, char* sta_password,
               unsigned int sta_password_len);
}  // namespace web

#endif