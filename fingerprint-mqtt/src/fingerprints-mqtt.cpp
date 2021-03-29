#include "fingerprint-mqtt.h"
#include "setup.h"
#include "config.h"
#include "led.h"
#include "manage.h"

uint8_t id = 0;
uint8_t lastId = 0;

uint8_t lastConfidenceScore = 0;

unsigned long lastMQTTmsg = 0;

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
    mqttMessage["mode"] = "reading";

    uint8_t result = fingerprintReading();

    if (result == FINGERPRINT_OK)
    {
      mqttMessage["match"] = true;
      mqttMessage["state"] = "Matched";
      mqttMessage["id"] = lastId;
      mqttMessage["confidence"] = lastConfidenceScore;
      mqttPublish("Finger match");

      lastMQTTmsg = millis();
      delay(200);
    }
    else if (result == FINGERPRINT_NOTFOUND)
    {
      mqttMessage["match"] = false;
      mqttMessage["id"] = id;
      mqttMessage["state"] = "Not matched";
      mqttMessage["confidence"] = lastConfidenceScore;
      mqttPublish("Finger do not match");

      lastMQTTmsg = millis();
      delay(100);
    }
    else if (result == FINGERPRINT_NOFINGER)
    {
      if ((millis() - lastMQTTmsg) > MQTT_INTERVAL)
      {
        mqttMessage["match"] = false;
        mqttMessage["id"] = id;
        mqttMessage["state"] = "Waiting";
        mqttMessage["confidence"] = 0;
        mqttPublish("Waiting...");

        lastMQTTmsg = millis();
      }

      if ((millis() - lastMQTTmsg) < 0)
      {
        lastMQTTmsg = millis();
      }
    }
  }
}

uint8_t fingerprintReading()
{
  uint8_t p = fingerSensor.getImage();

  switch (p)
  {

  case FINGERPRINT_OK:

    Serial.println("Image taken");
    break;

  case FINGERPRINT_NOFINGER:

    return p;

  case FINGERPRINT_PACKETRECIEVEERR:

    Serial.println("Communication error");
    return p;

  case FINGERPRINT_IMAGEFAIL:

    Serial.println("Imaging error");
    return p;

  default:

    Serial.println("Unknown error");
    return p;
  }

  // OK success!

  p = fingerSensor.image2Tz();

  switch (p)
  {

  case FINGERPRINT_OK:

    Serial.println("Image converted");
    break;

  case FINGERPRINT_IMAGEMESS:

    Serial.println("Image too messy");
    return p;

  case FINGERPRINT_PACKETRECIEVEERR:

    Serial.println("Communication error");
    return p;

  case FINGERPRINT_FEATUREFAIL:

    Serial.println("Could not find fingerprint features");
    return p;

  case FINGERPRINT_INVALIDIMAGE:

    Serial.println("Could not find fingerprint features");
    return p;

  default:

    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  p = fingerSensor.fingerSearch();

  if (p == FINGERPRINT_OK)
  {
    Serial.println("Found a print match!");

    lastId = fingerSensor.fingerID;
    lastConfidenceScore = fingerSensor.confidence;
    led(LED_MATCH);
    return p;
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");
    lastConfidenceScore = fingerSensor.confidence;
    led(LED_WRONG);
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }
}


void callback(char *topic, byte *payload, unsigned int length)
{
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