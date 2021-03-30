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

  led(LED_READY);
}

void loop()
{
  localLoop();

  if (sensorMode == "reading")
  {
    uint8_t result = fingerprintReading();

    if (result == RESULT_MATCH)
    {
      match = true;
      sensorState = "matched";

      led(LED_MATCH);
      Serial.println("Fingerprint match found.");
      mqttPublish("Fingerprint match found.");

      delay(200);
    }
    else if (result == RESULT_WRONG)
    {
      match = false;
      sensorState = "wrong";

      led(LED_WRONG);
      Serial.println("Unknown fingerprint detected.");
      mqttPublish("Unknown fingerprint detected.");

      delay(200);
    }

    mqttPublish("Waiting...");
  }
}

uint8_t fingerprintReading()
{

  uint8_t result = fingerSensor.getImage();

  if (result != FINGERPRINT_OK)
  {
    Serial.printf("Reading error [%x]\n\n", result);
    return RESULT_WAIT;
  }

  result = fingerSensor.image2Tz();

  if (result != FINGERPRINT_OK)
  {
    Serial.printf("Reading error [%x]\n\n", result);
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

  int id;
  if (strcmp(topic, MODE_LEARNING) == 0)
  {
    char charArray[3];

    for (unsigned int i = 0; i < length; i++)
    {
      charArray[i] = payload[i];
    }

    id = atoi(charArray);

    if (id > 0 && id < 128)
    {
      Serial.println("Entering Learning mode");

      sensorMode = "learning";
      mqttMessage["mode"] = "learning";
      mqttMessage["id"] = id;
      mqttMessage["confidence"] = 0;
      mqttPublish("Learning");
      /*
      while (!getFingerprintEnroll())
        ;
*/
      Serial.println("Exiting Learning mode");

      sensorMode = "reading";
      mqttMessage["mode"] = "reading";

      id = 0;
    }
    else
    {
      Serial.println("Invalid Id");
    }

    Serial.println();
  }

  if (strcmp(topic, MODE_DELETE) == 0)
  {
    char charArray[3];

    for (unsigned int i = 0; i < length; i++)
    {
      charArray[i] = payload[i];
    }

    id = atoi(charArray);

    if (id > 0 && id < 128)
    {
      sensorMode = "deleting";
      mqttMessage["mode"] = "deleting";
      mqttMessage["id"] = id;
      mqttMessage["confidence"] = 0;
      mqttPublish("Deleting");

      Serial.println("Entering delete mode");
      /*
      while (!deleteFingerprint())
        ;
*/
      Serial.println("Exiting delete mode");

      delay(2000);

      sensorMode = "reading";
      mqttMessage["mode"] = "reading";

      id = 0;
    }

    Serial.println();
  }
}