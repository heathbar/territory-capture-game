# Territory Capture Game

This project covers the software for creating a territory capture game for Laser Tag or Airsoft.

## Projects

- [controller-esp32](controller-esp32) contains the controller code for an esp32. The controller runs a webserver + websockets that are used to control and monitor the game. It communicates with the other stations using ESP NOW. 
- [controller-webui](controller-webui) contains an Angular application that is hosted on the controller. This application is used to control and monitor the game.
- [station-esp32](station-esp32) contains the code for an esp32 that runs each box that can be captured.
