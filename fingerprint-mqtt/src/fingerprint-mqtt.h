#include <stdint.h>
typedef unsigned char byte;

#include <Arduino.h>
#include "LittleFS.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_Fingerprint.h>

void setupDevices();
void readConfig();
void setupWifi();
void saveConfig();
void setupMqtt();
void saveConfig();
void readConfig();
void saveConfigCallback();
void setupMqtt();
void setupDevices();
void setupWifi();
uint8_t getFingerprintId();
uint8_t getFingerprintEnroll();
uint8_t deleteFingerprint();
void publish();
void callback(char *topic, byte *payload, unsigned int length);
void ledFinger();
void ledMatch();
void ledWrong();
void ledReady();
void ledWait();
