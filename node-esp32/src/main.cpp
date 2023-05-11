#include <Arduino.h>
#define FASTLED_ALL_PINS_HARDWARE_SPI

#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <FastLED.h>
#include "button.h"

#define NUM_LEDS 20

#define LED_DATA_PIN 23
#define LED_CLOCK_PIN 18

#define BTN_RED_GND 14
#define BTN_RED_INP 27

#define BTN_BLU_GND 13
#define BTN_BLU_INP 12

CRGB leds[NUM_LEDS];

WiFiManagerParameter node_name("node_name", "Enter the name of this node here", "charlie", 16);
WiFiManagerParameter mqtt_host("mqtt_host", "Enter the mqtt server here", "192.168.4.1", 32);

#define STATE_NEUTRAL 0
#define STATE_RED 1
#define STATE_BLUE 2
#define STATE_ENDGAME 10

int state = STATE_NEUTRAL;

#define USE_DEFAULT_SPI_PORT 1

const String client_id = String("laser-tag-") + node_name.getValue() + "-" + String(random(1000, 10000));
const String state_topic = String("state/") + node_name.getValue();
const String ctrl_topic = "ctrl";

// wifi_interface_t current_wifi_interface;
WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE 50
char msg[MSG_BUFFER_SIZE];

Button red_btn(BTN_RED_INP);
Button blu_btn(BTN_BLU_INP);

void mqtt_callback(char *topic, byte *payload, unsigned int length);
void mqtt_reconnect();
void publish(int state);
void animate(CRGB color, bool reverse);

void setup()
{
  pinMode(BTN_RED_GND, OUTPUT);
  pinMode(BTN_BLU_GND, OUTPUT);

  digitalWrite(BTN_RED_GND, LOW);
  digitalWrite(BTN_BLU_GND, LOW);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  WiFiManager wm;

  wm.addParameter(&node_name);
  wm.addParameter(&mqtt_host);

  bool res = wm.autoConnect((String("LaserTag-") + node_name.getValue()).c_str(), "L4S3RZ!!!"); // password protected ap

  if (!res)
  {
    Serial.println("Failed to connect");
    wm.resetSettings();
    // ESP.restart();
  }
  else
  {
    Serial.println("connected.");
    // esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_LR);
  }

  // Extra GND pin
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  mqtt.setServer(mqtt_host.getValue(), 1883);
  mqtt.setCallback(mqtt_callback);

  FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLOCK_PIN, BGR>(leds, NUM_LEDS);
}

void loop()
{

  if (!mqtt.connected())
  {
    mqtt_reconnect();
  }
  mqtt.loop();

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

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  switch ((char)payload[0])
  {
    case '0':
      Serial.println("RESET");
      state = STATE_NEUTRAL;
      publish(state);
      break;

    case '1':
      Serial.println("ENDGAME");
      state = STATE_NEUTRAL;
      publish(state);
      break;
  }
}

void mqtt_reconnect()
{
  while (!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (mqtt.connect(client_id.c_str()))
    {
      Serial.println("connected");
      mqtt.subscribe(ctrl_topic.c_str());
      delay(100);
      publish(state);

      for (int i = 0; i < 3; i++)
      {
        FastLED.showColor(CRGB::Green);
        delay(200);
        FastLED.showColor(CRGB::Black);
        delay(200);
      }
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publish(int state)
{
  mqtt.publish(state_topic.c_str(), String(state).c_str());
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