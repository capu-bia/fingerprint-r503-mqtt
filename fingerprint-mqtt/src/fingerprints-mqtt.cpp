#include "fingerprint-mqtt.h"
#include "setup.h"
#include "config.h"
#include "led.h"
#include "manage.h"

#define RESULT_MATCH 0
#define RESULT_WRONG 1

#define RESULT_WAIT 2

void setup()
{
  setupDevices();
  readConfig();
  setupWifi();
  saveConfig();
  mqttSetup(callback);
}

void loop()
{
  localLoop();

  if (sensorMode == MODE_READING)
  {
    resetMessage();
    mqttPublish("Waiting...");

    uint8_t result = fingerprintReading();

    if (result == RESULT_MATCH)
    {
      match = true;
      sensorState = STATE_MATCH;

      led(LED_MATCH);
      Serial.println("Fingerprint match found.");
      mqttPublish("Fingerprint match found.");

      delay(200);
    }
    else if (result == RESULT_WRONG)
    {
      match = false;
      sensorState = STATE_WRONG;

      led(LED_WRONG);
      Serial.println("Unknown fingerprint detected.");
      mqttPublish("Unknown fingerprint detected.");

      delay(200);
    }
  }
}

uint8_t fingerprintReading()
{
  uint8_t result = fingerSensor.getImage();

  if (result != FINGERPRINT_OK)
  {
    //Serial.printf("Reading error [%x]\n", result);
    return RESULT_WAIT;
  }

  result = fingerSensor.image2Tz();

  if (result != FINGERPRINT_OK)
  {
    Serial.printf("Reading error [%x]\n", result);
    return RESULT_WAIT;
  }

  Serial.println("Fingerprint image converted");

  result = fingerSensor.fingerSearch();

  if (result == FINGERPRINT_OK)
  {
    Serial.println("Fingerprint found");
    fingerprintId = fingerSensor.fingerID;
    confidence = fingerSensor.confidence;

    return RESULT_MATCH;
  }

  if (result == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Fingerprint unknown");
    fingerprintId = 0;
    confidence = fingerSensor.confidence;

    return RESULT_WRONG;
  }

  Serial.printf("Converting error [%x]\n\n", result);

  return RESULT_WAIT;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  boardLedOff();

  DynamicJsonDocument body(1024);
  deserializeJson(body, payload);

  uint8_t newFingerprintId = body["fingerprintId"];

  if (strcmp(topic, learnTopic) == 0)
  {
    Serial.printf("Learning id %d\n", fingerprintId);

    if (newFingerprintId < 10 || newFingerprintId > 200)
    {
      sensorMode = MODE_LEARNING;
      sensorState = STATE_ERROR;
      fingerprintId = newFingerprintId;
      match = false;
      mqttPublish("Invalid fingerprintId (10-200 for userId 1-20)");

      delay(1000);

      sensorMode = MODE_READING;
      sensorState = STATE_WAIT;
      fingerprintId = 0;
      mqttPublish("Waiting...");

      delay(1000);

      return;
    }

    sensorMode = MODE_LEARNING;
    sensorState = STATE_WAIT;
    fingerprintId = newFingerprintId;

    mqttPublish("Start learning...");

    processEnroll(newFingerprintId);

    delay(1000);

    Serial.println("Exit learning...");
    sensorMode = MODE_READING;

    return;
  }

  if (strcmp(topic, deleteTopic) == 0)
  {
    Serial.printf("Deleting id %d\n", fingerprintId);

    if (newFingerprintId < 10 || newFingerprintId > 200)
    {
      sensorMode = MODE_DELETING;
      sensorState = STATE_ERROR;
      fingerprintId = newFingerprintId;
      match = false;
      mqttPublish("Invalid fingerprintId (10-200 for userId 1-20)");

      delay(1000);

      sensorMode = MODE_READING;
      sensorState = STATE_WAIT;
      fingerprintId = 0;
      mqttPublish("Waiting...");

      delay(1000);

      return;
    }

    sensorMode = MODE_DELETING;
    sensorState = STATE_WAIT;
    fingerprintId = newFingerprintId;

    mqttPublish("Start deleting...");
    delay(1000);
    processDelete(newFingerprintId);

    Serial.println("Exit deleting...");
    sensorMode = MODE_READING;

    return;
  }
}