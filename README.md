# Fingerprint - R503

## What is this?

This a firmware for arduino like devices with integrated WIFI adapter, based on ESP8266 board, like the [Wemos D1 Mini](https://amzn.to/3fP5Pir).

Connect one of these board to a [R503 Fingerprint capacitive sensor](https://amzn.to/31WWdd6) and you can send commands via MQTT protocol to an IoT hub.

This project is made with Home Assistant integration in mind, but it can be used to send data to any MQTT broker.

## Features
* Simple configuration via WIFI
* Detect different users and different user's fingerprints
* Detect fingerprint mismatch
* Add and delete fingerprints via MQTT
* Led color code feedback
* Led signalling via MQTT


## What do you need?

### Hardware

1. Home Assistant with MQTT broker running (or another MQTT broker running on your LAN)
2. A fingerprint sensor model [R503](https://amzn.to/31WWdd6)
3. An ESP8266 board like the [Wemos D1 mini](https://amzn.to/3fP5Pir)

### Software

## Connections
<img src="doc/connections.jpg" width="650">


