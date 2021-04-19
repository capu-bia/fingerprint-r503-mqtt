# Fingerprint - R503

## What is this?

This a firmware for arduino like devices with integrated WIFI adapter, based on ESP8266 board, like the [Wemos D1 Mini](https://amzn.to/3fP5Pir).

Connect one of these board to a [R503 Fingerprint capacitive sensor](https://amzn.to/31WWdd6) and you can send commands via MQTT protocol to an IoT hub.

This project is made with [Home Assistant](https://www.home-assistant.io/) integration in mind, but it can be used to only interact with any MQTT broker.

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

1. [Home Assistant](https://www.home-assistant.io/) with MQTT broker running (or another MQTT broker running on your LAN)
2. A fingerprint sensor model [R503](https://amzn.to/31WWdd6)
3. An ESP8266 board like the [Wemos D1 mini](https://amzn.to/3fP5Pir)

### Software

Interaction will be done sending/receiving MQTT messages to a borker. Most used broker is called [Mosquitto](https://mosquitto.org/) and can be installed as [Home Assistant integration](https://www.home-assistant.io/integrations/mqtt/).

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

After flash and reboot, when no configuration is found the Wemos will tun on a WiFi network called "Fingerprint-Setup":

<img src="doc/wifi-ap.png" width="350">

Use a mobile phone or PC to connect to that WiFi network. A configuration page will popup:

<img src="doc/wifi-home.png" width="350">

Fill in all required fields:

* SSID name of your WiFi network
* WiFi password
* MQTT host: broker hostname or IP (use Home Assistant address if you are using it's Mosquitto add-on)
* MQTT port: broker port (usually 1883)
* MQTT username (configure an user in Mosquitto or let it be got from Home Assistant users)
* MQTT password (the password)
* Gate name: the name of *this* sensor (to distinguish from other devices on the same broker)

NB: do NOT use any special chars in the gate name field: only letters and numbers, no spaces or other things.

<img src="doc/wifi-config.png" width="350">

After saving, Wemos will reboot and will try to connect to WiFi network. On successful, it will flash blue led on sensor.

### Reset

If you mistyped some data or you want to reset device, do:

* tun on device
* wait 10 seconds
* press the reset button
* wait 5 seconds
* press again reset button

Device will reboot in config mode and Fingerprint-Setup WiFi network will come back again.

NB: reset will *NOT* reset any registered fingerprints on the sensor.

## Integration

Everything will be controlled sending and receiving MQTT messages. If you are using [Home Assistant](https://www.home-assistant.io/) you can both send and receive messages in:

**Configuration -> Integration -> MQTT - CONFIGURE**

Then you can set topic to send to json messages, and topic to listen to for incoming messages.

Topics must include the given *gate* name, following this table:


| Topic                           | Direction | Purpose                                     |
|---------------------------------|-----------|---------------------------------------------|
| /fingerprint/*gate*/**learn**   | outgoing  | Start the learning new fingerprint process  |
| /fingerprint/*gate*/**delete**  | outgoing  | Delete a previous learned fingerprint       |
| /fingerprint/*gate*/**status**  | incoming  | Current sensor status and messages          |

For example, if gate is "main":

<img src="doc/ha-mqtt.png" width="400">

### Learn

To initiate a learning process, send this message to the learn topic:

```
{
  "fingerPrintId": *number*
}
```

Where *number* is the fingerprint identification number, from 10 to 200. Suggested convention is to use intervals of 10 id's to match users. So, user 1 will have fingerprint id's from 10 to 19, user 2 from 20 to 29 and so on. Internally, a division by 10 will be done, so user calculation will be ready made.

The process require a fingerprint snapshot and than a match. So, the steps will be:

* Wait for purple led flashing
* Put the finger on the sensor
* Wait until led turns off
* Remove finger
* Wait for purple led flashing again
* Put same finger on the sensor
* Wait until led turns off
* Remove finger
* If successful, a three times blue flash will confirm

To programmatically follow the learning process read the *status* topic messages.

### Status

Status topic will be used to both inform user about learning/deleting process and current fingerprint matching status.

At idle, sensor will continuosly look for a fingerprint. Result will be reported in a message like:

```
{
    "message": "Waiting...",
    "state": "wait",
    "mode": "reading",
    "match": false,
    "fingerprintId": 0,
    "userId": 0,
    "confidence": 0,
    "gate": "main"
}
```

| Field         | Purpose                                                     |
|---------------|-------------------------------------------------------------|
| message       | Human readable message, useful when learning                |
| state         | Current sensor state, see below for values                  |
| mode          | If the sensor is reading, learning or deleting              |
| match         | True if a fingerprit has been recognized                    |
| fingerprintId | The recognized fingerprint                                  |
| userId        | The calculated userId, based on fingerprintId/10 convention |
| confidence    | How much the recognition is reliable                        |
| gate          | Current configured gate name                                |


State field can be:

* error: something is not working
* wrong: a wrong fingerprint has be detected, can be used as alarm trigger
* match: a recognized fingerprint has been detected
* wait: fingerprint sensor is waiting for a finger

Example, a matching message (slow blue led feedback):

```
{
    "message": "Fingerprint match found.",
    "state": "match",
    "mode": "reading",
    "match": true,
    "fingerprintId": 10,
    "userId": 1,
    "confidence": 98,
    "gate": "main"
}
```

A fingerprint has been detected but not recognized. An unknow hand to the sensor (red flashing led feedback):

```
{
    "message": "Unknown fingerprint detected.",
    "state": "wrong",
    "mode": "reading",
    "match": false,
    "fingerprintId": 0,
    "userId": 0,
    "confidence": 8,
    "gate": "main"
}
```

### Delete

To delete a fingerprint, send this message to the delete topic:

```
{
  "fingerPrintId": *number*
}
```

Where *number* is the fingerprint identification number to delete. Resetting device will not delete fingerprint because images are stored the sensor itself and not in the ESP firmware.

## Suggestion and contributions

If you have suggestions or ideas, please use [Issues](https://github.com/vinz486/fingerprint-r503-mqtt/issues) tab to contact me.


