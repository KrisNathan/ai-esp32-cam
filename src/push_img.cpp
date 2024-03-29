#include "push_img.hpp"
#include <HTTPClient.h>
#include <ArduinoJson.h>

void push_img(String base64_image) {
  JsonDocument doc;
  doc["img"] = base64_image;
  String json_string;
  serializeJson(doc, json_string);
  
  HTTPClient client;
  client.begin("http://ecam.knathanael.com/image_upload");
  client.addHeader("Content-Type", "application/json");
  int response_code = client.POST(json_string);
  Serial.printf("push_img: %d\r\n", response_code);
  client.end();
}