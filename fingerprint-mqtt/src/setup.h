#define SSID_FOR_SETUP "Fingerprint-Setup"
#define HOSTNAME "fingerprint-mqtt"

#define STATE_TOPIC "/fingerprint/status"
#define MODE_LEARNING "/fingerprint/learn"
#define MODE_READING "/fingerprint/read"
#define MODE_DELETE "/fingerprint/delete"
#define AVAILABILITY_TOPIC "/fingerprint/available"

#define MQTT_MAX_PACKET_SIZE 256
#define MQTT_INTERVAL 500
#define SENSOR_TX 12 // gren D5
#define SENSOR_RX 14 // yellow D6
#define SENSOR_TOUCH 13 // blue D0
#define CONFIG_FILE "/config.json"

#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

extern DynamicJsonDocument mqttMessage;
extern Adafruit_Fingerprint fingerSensor;

extern String lastSensorMode;
extern String sensorMode;
extern String lastSensorState;
extern String sensorState;

void mqttSetup(void (*callback)(char *topic, byte *payload, unsigned int length));
void mqttConnect();
void localLoop();
void mqttPublish();

void setupDevices();
void setupTouch();
void setupWifi();

void saveConfig();
void saveConfigCallback();
void readConfig();
