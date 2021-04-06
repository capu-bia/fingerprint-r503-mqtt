#define SSID_FOR_SETUP "Fingerprint-Setup"
#define HOSTNAME "fingerprint-mqtt"

#define TOPIC_STATUS "/fingerprint/status"
#define TOPIC_LEARN "/fingerprint/learn"
#define TOPIC_READ "/fingerprint/read"
#define TOPIC_DELETE "/fingerprint/delete"
#define TOPIC_AVAILABLE "/fingerprint/available"

#define MODE_READING "reading"
#define MODE_LEARNING "learning"
#define MODE_DELETING "deleting"

#define STATE_OK "ok"
#define STATE_ERROR "error"
#define STATE_WRONG "wrong"
#define STATE_MATCH "match"
#define STATE_WAIT "wait"

#define MQTT_MAX_PACKET_SIZE 256
#define MQTT_INTERVAL 500
#define SENSOR_TX 12 // gren D5
#define SENSOR_RX 14 // yellow D6
#define SENSOR_TOUCH 16 // blue D0

#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

extern Adafruit_Fingerprint fingerSensor;

extern uint8_t fingerprintId;
extern bool match;
extern uint8_t userId;
extern uint8_t confidence;
extern String sensorMode;
extern String sensorState;
extern String userMessage;

extern String lastSensorMode;
extern String lastSensorState;

extern char mqttHost[32];
extern char mqttPort[6];
extern char mqttUsername[16];
extern char mqttPassword[16];
extern char deviceGateId[32];

void mqttSetup(void (*callback)(char *topic, byte *payload, unsigned int length));
void mqttConnect();
void localLoop();
void mqttPublish(String message);
void resetMessage();

void setupDevices();
void setupTouch();
