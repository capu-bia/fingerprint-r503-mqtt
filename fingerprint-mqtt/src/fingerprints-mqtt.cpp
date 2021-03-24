#include "fingerprint-mqtt.h"
#include "setup.h"

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

  ledReady();
}

void loop()
{
  mqttKeep();
  
  if (sensorMode == "reading")
  {
    mqttMessage["mode"] = "reading";

    uint8_t result = getFingerprintId();

    if (result == FINGERPRINT_OK)
    {
      mqttMessage["match"] = true;
      mqttMessage["state"] = "Matched";
      mqttMessage["id"] = lastId;
      mqttMessage["confidence"] = lastConfidenceScore;
      mqttPublish();

      lastMQTTmsg = millis();
      delay(200);
    }
    else if (result == FINGERPRINT_NOTFOUND)
    {
      mqttMessage["match"] = false;
      mqttMessage["id"] = id;
      mqttMessage["state"] = "Not matched";
      mqttMessage["confidence"] = lastConfidenceScore;
      mqttPublish();

      lastMQTTmsg = millis();
      delay(100);
    }
    else if (result == FINGERPRINT_NOFINGER)
    {
        Serial.println("nofing");

      if ((millis() - lastMQTTmsg) > MQTT_INTERVAL)
      {
        mqttMessage["match"] = false;
        mqttMessage["id"] = id;
        mqttMessage["state"] = "Waiting";
        mqttMessage["confidence"] = 0;
        mqttPublish();

        lastMQTTmsg = millis();
      }

      if ((millis() - lastMQTTmsg) < 0)
      {
        lastMQTTmsg = millis();
      }
    }
  }

  Serial.println("Loop");

  mqttLoop();
}

uint8_t getFingerprintId()
{
  uint8_t p = fingerSensor.getImage();

  switch (p)
  {

  case FINGERPRINT_OK:

    Serial.println("Image taken");
    break;

  case FINGERPRINT_NOFINGER:

    Serial.println("No finger detected");
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
    ledMatch();
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
    ledWrong();
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }
}

uint8_t getFingerprintEnroll()
{
  int p = -1;

  sensorMode = "learning";
  mqttMessage["mode"] = "learning";
  mqttMessage["id"] = id;
  mqttMessage["state"] = "Place finger...";
  mqttMessage["confidence"] = 0;
  mqttPublish();

  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);

  while (p != FINGERPRINT_OK)
  {
    p = fingerSensor.getImage();

    switch (p)
    {

    case FINGERPRINT_OK:

      Serial.println("Image taken");
      ledFinger();
      break;

    case FINGERPRINT_NOFINGER:

      Serial.print(".");
      ledWait();
      break;

    case FINGERPRINT_PACKETRECIEVEERR:

      Serial.println("Communication error");
      break;

    case FINGERPRINT_IMAGEFAIL:

      Serial.println("Imaging error");
      break;

    default:

      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = fingerSensor.image2Tz(1);

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

  sensorMode = "learning";
  mqttMessage["mode"] = "learning";
  mqttMessage["id"] = id;
  mqttMessage["state"] = "Remove finger...";
  mqttMessage["confidence"] = 0;
  mqttPublish();

  Serial.println("Remove finger");
  delay(2000);

  p = 0;

  while (p != FINGERPRINT_NOFINGER)
  {
    p = fingerSensor.getImage();
  }

  Serial.print("Id ");
  Serial.println(id);

  p = -1;

  sensorMode = "learning";
  mqttMessage["mode"] = "learning";
  mqttMessage["id"] = id;
  mqttMessage["state"] = "Place same finger again...";
  mqttMessage["confidence"] = 0;
  mqttPublish();

  Serial.println("Place same finger again");

  while (p != FINGERPRINT_OK)
  {
    p = fingerSensor.getImage();

    switch (p)
    {

    case FINGERPRINT_OK:

      Serial.println("Image taken");
      ledFinger();
      break;

    case FINGERPRINT_NOFINGER:

      Serial.print(".");
      ledWait();
      break;

    case FINGERPRINT_PACKETRECIEVEERR:

      Serial.println("Communication error");
      break;

    case FINGERPRINT_IMAGEFAIL:

      Serial.println("Imaging error");
      break;

    default:

      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = fingerSensor.image2Tz(2);

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
  Serial.print("Creating model for #");
  Serial.println(id);

  p = fingerSensor.createModel();

  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("Id ");
  Serial.println(id);

  p = fingerSensor.storeModel(id);

  if (p == FINGERPRINT_OK)
  {
    sensorMode = "learning";
    mqttMessage["mode"] = "learning";
    mqttMessage["id"] = id;
    mqttMessage["state"] = "Success, stored";
    mqttMessage["confidence"] = 0;
    mqttPublish();

    Serial.println("Stored!");
    return true;
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }
}

uint8_t deleteFingerprint()
{
  uint8_t p = -1;

  p = fingerSensor.deleteModel(id);

  if (p == FINGERPRINT_OK)
  {
    sensorMode = "deleting";
    mqttMessage["mode"] = "deleting";
    mqttMessage["id"] = id;
    mqttMessage["state"] = "Deleted";
    mqttMessage["confidence"] = 0;
    mqttPublish();

    ledWrong();

    return true;
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {

    Serial.println("Could not delete in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
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
      mqttPublish();

      while (!getFingerprintEnroll())
        ;

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
      mqttPublish();

      Serial.println("Entering delete mode");

      while (!deleteFingerprint())
        ;

      Serial.println("Exiting delete mode");

      delay(2000);

      sensorMode = "reading";
      mqttMessage["mode"] = "reading";

      id = 0;
    }

    Serial.println();
  }
}

// mode(1-6),delay(1-255),color(1-Red/2-Blue/3-Purple),times(1-255)

void ledFinger()
{
  fingerSensor.led_control(1, 100, 2, 1);
}

void ledMatch()
{
  fingerSensor.led_control(1, 150, 2, 1);
}

void ledWrong()
{
  fingerSensor.led_control(1, 30, 1, 2);
}

void ledReady()
{
  fingerSensor.led_control(2, 150, 2, 1);
}

void ledWait()
{
  fingerSensor.led_control(1, 15, 3, 1);
}
