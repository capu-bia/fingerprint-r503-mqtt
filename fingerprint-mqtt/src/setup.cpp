#include "setup.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "LittleFS.h"
#include "Adafruit_Fingerprint.h"

DynamicJsonDocument mqttMessage(MQTT_MAX_PACKET_SIZE);
SoftwareSerial swSerial(SENSOR_TX, SENSOR_RX);
Adafruit_Fingerprint fingerSensor = Adafruit_Fingerprint(&swSerial);

String sensorMode = "reading";

String lastSensorMode;
String lastSensorState;
String sensorState;

int touchVal;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
char mqttBuffer[MQTT_MAX_PACKET_SIZE];

char mqttHost[32] = "homeassistant.local";
WiFiManagerParameter customMqttHost("mqttHost", "MQTT host", mqttHost, 32);

char mqttPort[6] = "1883";
WiFiManagerParameter customMqttPort("mqttPort", "MQTT port", mqttPort, 6);

char mqttUsername[16] = "mqtt";
WiFiManagerParameter customMqttUsername("mqttUsername", "MQTT username", mqttUsername, 16);

char mqttPassword[16] = "mqtt";
WiFiManagerParameter customMqttPassword("mqttPassword", "MQTT password", mqttPassword, 16);

char gateId[32] = "main";
WiFiManagerParameter customGateId("gateId", "Gate name", gateId, 32);

bool shouldSaveConfig = false;

void mqttPublish()
{
    const char *state = mqttMessage["state"];
    String sensorState = String(state);

    int id = mqttMessage["id"];
    mqttMessage["user"] = id / 10;

    if ((sensorMode != lastSensorMode) || (sensorState != lastSensorState))
    {
        Serial.println("Publishing state... mode: " + sensorMode + " state: " + sensorState);

        lastSensorMode = sensorMode;
        lastSensorState = sensorState;

        size_t mqttMessageSize = serializeJson(mqttMessage, mqttBuffer);
        client.publish(STATE_TOPIC, mqttBuffer, mqttMessageSize);
        Serial.println(mqttBuffer);
    }
}

void setupWifi()
{
    Serial.println("Configuration check...");

    WiFiManager wifiManager;
    //wifiManager.resetSettings();

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

void setupDevices()
{
    Serial.begin(9600);

    while (!Serial)
        ;
    delay(100);

    Serial.println("\n\nWelcome to Fingerprint-MQTT sensor");

    fingerSensor.begin(57600);
    delay(5);

    Serial.print("Looking for sensor...");

    if (fingerSensor.verifyPassword())
    {
        Serial.println("ok");
    }
    else
    {
        Serial.println("KO.\nSensor not found: check serial connection on green/yellow cables.");

        delay(3000);
        ESP.restart();
        delay(5000);
    }

    fingerSensor.getParameters();
    Serial.print(F("Status : 0x"));
    Serial.println(fingerSensor.status_reg, HEX);
    Serial.print(F("Sys ID : 0x"));
    Serial.println(fingerSensor.system_id, HEX);
    Serial.print(F("Capacity: "));
    Serial.println(fingerSensor.capacity);
    Serial.print(F("Security level: "));
    Serial.println(fingerSensor.security_level);
    Serial.print(F("Device address: "));
    Serial.println(fingerSensor.device_addr, HEX);
    Serial.print(F("Packet len: "));
    Serial.println(fingerSensor.packet_len);
    Serial.print(F("Baud rate: "));
    Serial.println(fingerSensor.baud_rate);

    fingerSensor.getTemplateCount();

    if (fingerSensor.templateCount == 0)
    {
        Serial.print("Sensor doesn't contain any fingerprint data");
    }
    else
    {
        Serial.println("Waiting for valid finger");
        Serial.print("Sensor contains ");
        Serial.print(fingerSensor.templateCount);
        Serial.println(" templates");
    }

    //pinMode(SENSOR_TOUCH, INPUT);
    //attachInterrupt(digitalPinToInterrupt(SENSOR_TOUCH), setupTouch, CHANGE);
}

ICACHE_RAM_ATTR void setupTouch()
{
    Serial.println("Touch!");
}

void mqttSetup(void (*callback)(char *topic, byte *payload, unsigned int length))
{
    client.setServer(mqttHost, atoi(mqttPort));
    client.setCallback(callback);

    mqttMessage["gate"] = gateId;
    mqttMessage["mode"] = "reading";
    mqttMessage["match"] = false;
    mqttMessage["state"] = "Not matched";
    mqttMessage["id"] = 0;
    mqttMessage["user"] = 0;
    mqttMessage["confidence"] = 0;

    mqttConnect();
}

void mqttConnect()
{
    while (!client.connected())
    {
        Serial.print("Connecting to MQTT ");
        Serial.print(mqttHost);
        Serial.print("...");

        if (client.connect(HOSTNAME, mqttUsername, mqttPassword, AVAILABILITY_TOPIC, 1, true, "offline"))
        {
            Serial.println("connected");

            client.publish(AVAILABILITY_TOPIC, "online");
            client.subscribe(MODE_LEARNING);
            client.subscribe(MODE_READING);
            client.subscribe(MODE_DELETE);
        }
        else
        {
            Serial.print("failed, rc: ");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");

            delay(5000);
        }
    }
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
    strcpy(gateId, customGateId.getValue());

    doc["mqttHost"] = mqttHost;
    doc["mqttPort"] = mqttPort;
    doc["mqttUsername"] = mqttUsername;
    doc["mqttPassword"] = mqttPassword;
    doc["gateId"] = gateId;

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
                    strcpy(gateId, doc["gateId"]);
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

void localLoop()
{
    mqttConnect();

    delay(200);

    client.loop();
}
