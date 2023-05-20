#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include "message-types.h"

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);

    DynamicJsonDocument doc(256);
    deserializeJson(doc, (char*)data);

    if (doc["message"] == "0")
    {
      reset_message msg;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &msg, sizeof(msg));
      if (result != ESP_OK)
      {
        Serial.println("Error sending the data");
      }
    }    
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
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