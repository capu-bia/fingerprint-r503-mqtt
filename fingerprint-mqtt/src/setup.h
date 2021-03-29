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
#define SENSOR_TOUCH 16 // blue D0

#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

extern DynamicJsonDocument mqttMessage;
extern Adafruit_Fingerprint fingerSensor;

extern String lastSensorMode;
extern String sensorMode;
extern String lastSensorState;
extern String sensorState;

extern char mqttHost[32];
extern char mqttPort[6];
extern char mqttUsername[16];
extern char mqttPassword[16];
extern char gateId[32];

void mqttSetup(void (*callback)(char *topic, byte *payload, unsigned int length));
void mqttConnect();
void localLoop();
void mqttPublish(String message);

void setupDevices();
void setupTouch();
