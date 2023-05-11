# Controller - Territory Capture Game

## Setup

### OS

Default Raspbian OS

### Wireless Access Point

The following tutorial will setup a virtual device named `ap0` on the respberry pi that broadcasts a wireless access point, while still allowing the wlan0 device to be connected to another access point for internet/intranet access to/from the raspberry pi.

[Setup Wireless Access Point](https://www.paulligocki.com/wireless-access-point-raspberry-pi-zero-w/)


### Mosquitto

MQTT software install

```bash
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto.service
```

Enable remote connections
```
sudo nano /etc/mosquitto/mosquitto.conf
```

Add this to the bottom
```
listener 1883
allow_anonymous true
```

Restart
```
sudo systemctl restart mosquitto.service
```

### Controller

TBD