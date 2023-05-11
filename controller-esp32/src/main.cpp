#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "EmbeddedMqttBroker.h"

using namespace mqttBrokerName;

const char *ssid = "LaserTag";
const char *password = "L4S3RZ!!!";


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

uint16_t mqttPort = 1883;
MqttBroker broker(mqttPort);


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);

    DynamicJsonDocument doc(256);
    deserializeJson(doc, (char*)data);

    PublishMqttMessage m;
    m.setTopic(doc["topic"].as<String>());
    m.setPayLoad(doc["message"].as<String>());
    broker.publishMessage(&m);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void onMqttMessageReceived(PublishMqttMessage m)
{
  DynamicJsonDocument json(1024);
  json["topic"] = m.getTopic().getTopic();
  json["message"] = m.getTopic().getPayLoad();
  String s;
  serializeJson(json, s);
  ws.textAll(s);
}


void setup(){
  
  /**
   * @brief To see outputs of broker activity 
   * (message to publish, new client's id etc...), 
   * set your core debug level higher to NONE (I recommend INFO level).
   * More info: @link https://github.com/alexCajas/EmbeddedMqttBroker @endlink
   */

  Serial.begin(115200);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
    
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.println("simple mqtt broker");
  Serial.print("Starting up ");
  Serial.println(ssid);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  ws.onEvent(onEvent);
  server.addHandler(&ws);


  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
    
  // server.on("/api/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   return api.getWifi(request);
  // });

  // server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   return api.getState(request);
  // });

  // server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   return api.reset(request);
  // });
  
  // // Route to load style.css file
  // server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(SPIFFS, "/style.css", "text/css");
  // });

  // // Route to set GPIO to HIGH
  // server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
  //   digitalWrite(ledPin, HIGH);    
  //   request->send(SPIFFS, "/index.html", String(), false, processor);
  // });
  
  // // Route to set GPIO to LOW
  // server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
  //   digitalWrite(ledPin, LOW);    
  //   request->send(SPIFFS, "/index.html", String(), false, processor);
  // });

  // Start server
  server.begin();
  Serial.println("HTTP server started");

  // Start the mqtt broker
  broker.setMaxNumClients(9); // set according to your system.
  broker.echoMessages(onMqttMessageReceived);
  broker.startBroker();
  Serial.println("MQTT broker started");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop()
{
  ws.cleanupClients();
}
