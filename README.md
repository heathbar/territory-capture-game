# Territory Capture Game

This project covers the software for creating a territory capture game for Laser Tag or Airsoft.

## Projects

- [controller-esp32](controller-esp32) contains the controller code for an esp32. The controller runs an MQTT server that is used to communicate with the other nodes. It also runs a webserver that is used to control and monitor the game.
- [controller-webui](controller-webui) contains an Angular application that is hosted on the controller. This application is used to control and monitor the game.
- [node-esp32](node-esp32) contains the code for an esp32 that runs each box that can be captured.
