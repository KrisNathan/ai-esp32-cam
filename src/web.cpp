#include "web.hpp"

namespace web {
char* DEFAULT_USERNAME = "user";
char* DEFAULT_PASSWORD = "user";
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

void setup_server(AsyncWebServer* server, bool* take_photo_state) {
  server->serveStatic("/", SPIFFS, "/")
      .setDefaultFile("index.html")
      .setAuthentication("user", "pass");
  ;

  // server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send_P(200, "text/html", index_html);
  // });

  // server->on("/set_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
  //   if (request->hasParam("ssid") && request->hasParam("pw")) {
  //     auto ssid = request->getParam("ssid");
  //     auto pw = request->getParam("pw");

  //     WiFi.softAPdisconnect();

  //     WiFi.begin(ssid->value(), pw->value());
  //     while (WiFi.status() != WL_CONNECTED) {
  //       delay(1000);
  //       Serial.print("Connecting to WiFi...: ");
  //       Serial.println(WiFi.status());
  //     }

  //     request->send_P(200, "text/plain", "Success.");
  //     return;
  //   }
  //   request->send_P(400, "text/plain", "Bad request.");
  // });

  // // dangerous driving
  // server->on("/capture", HTTP_GET, [take_photo_state](AsyncWebServerRequest*
  // request) {
  //   *take_photo_state = true;
  //   request->send_P(200, "text/plain", "Taking Photo");
  // });

  // server->on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest* request)
  // {
  //   request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  // });

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

void loop_server() {}

}  // namespace web