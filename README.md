# Fingerprint - R503

## What is this?

This a firmware for arduino like devices with integrated WIFI adapter, based on ESP8266 board, like the [Wemos D1 Mini](https://amzn.to/3fP5Pir).

Connect one of these board to a [R503 Fingerprint capacitive sensor](https://amzn.to/31WWdd6) and you can send commands via MQTT protocol to an IoT hub.

This project is made with Home Assistant integration in mind, but it can be used to only interact with any MQTT broker.

## Features
* Simple configuration via WIFI
* Detect different users and different user's fingerprints
* Detect fingerprint mismatch
* Add and delete fingerprints via MQTT
* Led color code feedback
* Led signalling via MQTT

Home Assistant can use messages to enable automations and trigger things like:

* Open a door using electric locks
* Enable/disable alarm system using different fingerprints
* Enable/disable access based on user and/or time
* Activate notificatons on unknown fingerprint

## What do you need?

### Hardware

1. Home Assistant with MQTT broker running (or another MQTT broker running on your LAN)
2. A fingerprint sensor model [R503](https://amzn.to/31WWdd6)
3. An ESP8266 board like the [Wemos D1 mini](https://amzn.to/3fP5Pir)

### Software

Interaction will be done sending/receiving MQTT messages to a borker. Most used broker is called Mosquitto and can be installed as Home Assistant integration.

## How to

### Connections
<img src="doc/connections.jpg" width="650">

* Red cable on 3.3 volt
* Black cable on ground
* Green cable on pin D5
* Yellow cable on pin D6

The other 2 sensor cables can be left unused.

### Flash

This project is not made using standard Arduino IDE. It's a PlatformIO project.

Connect the Wemos to a usb port and:

* Linux (and maybe Mac): type `$ ./install_into_usb_connected_wemos_d1.sh` (python3 and curl are needed)
* Windows: I don't know. Follow PlatformIO docs to flash under Windows machines.

### Configure
After flash and reboot, when no configuration is found the Wemos will tun on a WiFi netowrk called "Fingerprint-Setup". Use a mobile phone or PC to connect to that WiFi network a configuration page will popup:

