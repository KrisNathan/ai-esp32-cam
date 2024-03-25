#ifndef WEB_HPP
#define WEB_HPP

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>

#define FILE_PHOTO "/photo.jpg"

namespace web {
void setup_server(AsyncWebServer *server, bool *take_photo_state);
void loop_server();
}

#endif