#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "message-types.h"
#include "web-socket-handlers.h"

const char *ssid = "LaserTag";
const char *password = "laserface";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

void esp_now_register_peer();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *buf, int len);
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

 
void setup() {
  delay(2000); // wait for serial monitor to catch up
  Serial.begin(115200);
    
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
 
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.softAPIP());

  esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  uint8_t p;
  esp_wifi_get_protocol(WIFI_IF_AP, &p);
  Serial.println(p);
  esp_wifi_get_protocol(WIFI_IF_STA, &p);
  Serial.println(p);

  // esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  esp_now_register_peer();

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop()
{
  ws.cleanupClients();
  delay(2000);
}

void esp_now_register_peer()
{
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *buf, int len)
{
  if (buf[0] == MSG_STATION_BEACON)
  {
    Serial.println("Station Beacon recieved");
    station_beacon_message msg;
    memcpy(&msg, buf, sizeof(msg));

    uint8_t myAddress[6];
    sscanf(WiFi.macAddress().c_str(), "%X:%X:%X:%X:%X:%X", &myAddress[0], &myAddress[1], &myAddress[2], &myAddress[3], &myAddress[4], &myAddress[5]);

    controller_beacon_message beacon(myAddress);

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &beacon, sizeof(beacon));

    if (result != ESP_OK)
    {
      Serial.println("Error sending the data");
    }
  }
  else if (buf[0] == MSG_STATE)
  {
    Serial.print("State Received: ");
    state_message msg;
    memcpy(&msg, buf, sizeof(msg));
    Serial.println(String(msg.stationId) + ": " + msg.state);

    DynamicJsonDocument json(1024);
    json["stationId"] = msg.stationId;
    json["state"] = msg.state;
    String s;
    serializeJson(json, s);
    ws.textAll(s);
  }
}

