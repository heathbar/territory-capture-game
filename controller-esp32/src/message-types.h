#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

#include <Arduino.h>

const uint8_t MSG_CONTROL_BEACON = 0;
const uint8_t MSG_STATION_BEACON = 1;
const uint8_t MSG_RESET = 2;
const uint8_t MSG_STATE = 3;


struct controller_beacon_message {
  uint8_t type;
  uint8_t mac[6];

  controller_beacon_message() {}
  controller_beacon_message(uint8_t mac_addr[6])
  {
    type = MSG_CONTROL_BEACON;
    memcpy(mac, mac_addr, 6);
  }
};

struct station_beacon_message {
  uint8_t type;
  uint8_t mac[6];

  station_beacon_message() {}
  station_beacon_message(uint8_t mac_addr[6])
  {
    type = MSG_STATION_BEACON;
    memcpy(mac, mac_addr, 6);
  }
};

struct reset_message {
  uint8_t type;
  uint8_t data;

  reset_message(): type(MSG_RESET) {}
};

struct state_message {
  uint8_t type;
  uint8_t stationId;
  uint8_t state;

  state_message(): type(MSG_STATE) {}
  state_message(uint8_t id, uint8_t s): type(MSG_STATE), stationId(id), state(s) {}
};

#endif