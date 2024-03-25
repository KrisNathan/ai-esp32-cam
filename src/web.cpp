#include "web.hpp"

namespace web {
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align:center; }
    .vert { margin-bottom: 10%; }
    .hori{ margin-bottom: 0%; }
  </style>
</head>
<body>
  <div id="container">
    <h2>ESP32-CAM Last Photo</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="rotatePhoto();">ROTATE</button>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
    </p>
  </div>
  <div><img src="saved-photo" id="photo" width="70%"></div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.send();
  }
  function rotatePhoto() {
    var img = document.getElementById("photo");
    deg += 90;
    if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
    else{ document.getElementById("container").className = "hori"; }
    img.style.transform = "rotate(" + deg + "deg)";
  }
  function isOdd(n) { return Math.abs(n % 2) == 1; }
</script>
</html>)rawliteral";

void setup_server(AsyncWebServer *server, bool *take_photo_state) {
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server->on("/set_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid") && request->hasParam("pw")) {
      auto ssid = request->getParam("ssid");
      auto pw = request->getParam("pw");

      WiFi.softAPdisconnect();

      WiFi.begin(ssid->value(), pw->value());
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("Connecting to WiFi...: ");
        Serial.println(WiFi.status());
      }

      request->send_P(200, "text/plain", "Success.");
      return;
    }
    request->send_P(400, "text/plain", "Bad request.");
  });

  // dangerous driving
  server->on("/capture", HTTP_GET, [take_photo_state](AsyncWebServerRequest * request) {
    *take_photo_state = true;
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server->on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });

  server->begin();
}

void loop_server() {}

}