#include <Arduino.h>
#define FASTLED_ALL_PINS_HARDWARE_SPI

#include <WiFi.h>
#include <SPI.h>
#include <FastLED.h>
#include "button.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include "message-types.h"

#define STATION_ID 1
#define NUM_LEDS 20

#define LED_DATA_PIN 23
#define LED_CLOCK_PIN 18

#define BTN_RED_GND 14
#define BTN_RED_INP 27

#define BTN_BLU_GND 13
#define BTN_BLU_INP 12

uint8_t controllerAddress[] = { 0, 0, 0, 0, 0, 0 };
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

CRGB leds[NUM_LEDS];

#define STATE_NEUTRAL 0
#define STATE_RED 1
#define STATE_BLUE 2
#define STATE_ENDGAME 10

int state = STATE_NEUTRAL;

#define USE_DEFAULT_SPI_PORT 1

Button red_btn(BTN_RED_INP);
Button blu_btn(BTN_BLU_INP);

void addPeer(uint8_t mac[6]);
void locateController();
void publish(int state);
void animate(CRGB color, bool reverse);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);


void setup() {
  delay(2000); // wait for serial monitor
  Serial.begin(115200);
  
  FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLOCK_PIN, BGR>(leds, NUM_LEDS);
  
  pinMode(BTN_RED_GND, OUTPUT);
  pinMode(BTN_BLU_GND, OUTPUT);

  digitalWrite(BTN_RED_GND, LOW);
  digitalWrite(BTN_BLU_GND, LOW);


  WiFi.mode(WIFI_STA);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  FastLED.showColor({ 255, 255, 0});

  // esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("Locating Controller...");
  addPeer(broadcastAddress);
  while (controllerAddress[0] == 0)
  {
    locateController();
    Serial.print(".");
    delay(500);
  }

  FastLED.showColor({ 0, 255, 0});
  delay(500);
}
 
void loop() {
if (state != STATE_ENDGAME)
  {
    if (red_btn.isPressed())
    {
      state = STATE_RED;
      publish(state);
    }

    if (blu_btn.isPressed())
    {
      state = STATE_BLUE;
      FastLED.showColor(CRGB::Blue);
      publish(state);
    }

    switch (state)
    {
      case STATE_RED:
        animate(CRGB::Red, false);
        break;
      case STATE_NEUTRAL:
        FastLED.showColor(CRGB::Black);
        break;
      case STATE_BLUE:
        animate(CRGB::Blue, true);
        break;
      default:
        FastLED.showColor(CRGB::Black);
    }
  }
}


void addPeer(uint8_t mac[6])
{
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void locateController()
{
  uint8_t myAddress[6];
  sscanf(WiFi.macAddress().c_str(), "%X:%X:%X:%X:%X:%X", &myAddress[0], &myAddress[1], &myAddress[2], &myAddress[3], &myAddress[4], &myAddress[5]);
  station_beacon_message msg(myAddress);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &msg, sizeof(msg));

  if (result != ESP_OK) {
    Serial.println("Error sending the data");
    // char buf[50];
    // esp_err_to_name_r(result, buf, 50);
    // Serial.println(buf);
  }
}

void publish(int state)
{
  state_message m((uint8_t)STATION_ID, (uint8_t)state);
  esp_err_t result = esp_now_send(controllerAddress, (uint8_t *) &m, sizeof(m));

  // if (result == ESP_OK) {
  //   Serial.println("Sent with success");
  // }
  // else {
  //   Serial.println("Error sending the data");
  //   char buf[50];
  //   esp_err_to_name_r(result, buf, 50);
  //   Serial.println(buf);
  // }
}

void animate(CRGB color, bool reverse = false)
{
  int progress = millis() % 500;
  bool dir = false;


  for (int i = 0; i < NUM_LEDS; i++)
  {
    short black = progress / 50;

    bool condition = (i + black) % 10 < 5;

    if (reverse)
    {
      condition = (i + 10 - black) % 10 < 5;
    }

    if (condition)
    {
      leds[i] = CRGB::Black;
    }
    else
    {
      leds[i] = color;
    }
  }
  FastLED.show();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  if (status != ESP_NOW_SEND_SUCCESS)
  {
    // TODO: resend
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *buf, int len) {

  if (buf[0] == MSG_CONTROL_BEACON && controllerAddress[0] == 0)
  {
    Serial.println("Controller Beacon recieved");
    controller_beacon_message msg;
    memcpy(&msg, buf, sizeof(msg));
    memcpy(&controllerAddress, msg.mac, sizeof(msg.mac));

    addPeer(controllerAddress);
    publish(state);
  }
  else if (buf[0] == MSG_RESET)
  {
    state = STATE_NEUTRAL;
    publish(state);
  }
  else
  {
    Serial.println("Unknown message received");
  }
}