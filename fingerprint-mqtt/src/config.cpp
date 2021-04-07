#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include "config.h"
#include "setup.h"

bool shouldSaveConfig = false;

WiFiManagerParameter customMqttHost("mqttHost", "MQTT host", mqttHost, 32);
WiFiManagerParameter customMqttPort("mqttPort", "MQTT port", mqttPort, 6);
WiFiManagerParameter customMqttUsername("mqttUsername", "MQTT username", mqttUsername, 16);
WiFiManagerParameter customMqttPassword("mqttPassword", "MQTT password", mqttPassword, 16);
WiFiManagerParameter customGateId("deviceGateId", "Gate name", deviceGateId, 32);

void setupWifi()
{
    Serial.println("Configuration check...");

    WiFiManager wifiManager;

    wifiManager.addParameter(&customMqttHost);
    wifiManager.addParameter(&customMqttPort);
    wifiManager.addParameter(&customMqttUsername);
    wifiManager.addParameter(&customMqttPassword);
    wifiManager.addParameter(&customGateId);

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    if (!wifiManager.autoConnect(SSID_FOR_SETUP))
    {
        Serial.println("Failed to connect and hit timeout");

        delay(3000);
        ESP.restart();
        delay(5000);
    }
}

void saveConfigCallback()
{
    Serial.println("Setup complete.");
    shouldSaveConfig = true;
}

void saveConfig()
{
    if (!shouldSaveConfig)
    {
        Serial.println("No need to save config");
        return;
    }

    Serial.println("Saving configuration...");

    LittleFS.remove(CONFIG_FILE);

    File file = LittleFS.open(CONFIG_FILE, "w");

    if (!file)
    {
        Serial.println(F("Failed to create config file"));
        return;
    }

    StaticJsonDocument<512> doc;

    strcpy(mqttHost, customMqttHost.getValue());
    strcpy(mqttPort, customMqttPort.getValue());
    strcpy(mqttUsername, customMqttUsername.getValue());
    strcpy(mqttPassword, customMqttPassword.getValue());
    strcpy(deviceGateId, customGateId.getValue());

    doc["mqttHost"] = mqttHost;
    doc["mqttPort"] = mqttPort;
    doc["mqttUsername"] = mqttUsername;
    doc["mqttPassword"] = mqttPassword;
    doc["gateId"] = deviceGateId;

    if (serializeJson(doc, file) == 0)
    {
        Serial.println(F("Failed to write to config file"));
    }

    file.close();

    shouldSaveConfig = false;

    Serial.println("Config saved");
}

void readConfig()
{
    if (LittleFS.begin())
    {
        Serial.println("Mounted file system");

        if (LittleFS.exists(CONFIG_FILE))
        {
            Serial.println("Reading config file");

            File configFile = LittleFS.open(CONFIG_FILE, "r");

            if (configFile)
            {
                Serial.println("Opened config file");

                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, configFile);

                if (error)
                {
                    Serial.println(F("Failed to read file, using default configuration"));
                }
                else
                {
                    Serial.println("Loaded config:");
                    serializeJson(doc, Serial);

                    strcpy(mqttHost, doc["mqttHost"]);
                    strcpy(mqttPort, doc["mqttPort"]);
                    strcpy(mqttUsername, doc["mqttUsername"]);
                    strcpy(mqttPassword, doc["mqttPassword"]);
                    strcpy(deviceGateId, doc["gateId"]);
                }
            }

            configFile.close();
        }
    }
    else
    {
        Serial.println("Failed to mount FS");
    }
}
