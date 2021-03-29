#include "manage.h"
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include "setup.h"
#include "led.h"

#define IMAGE_ONE 1
#define IMAGE_TWO 2

bool processEnroll(int fingerprintId)
{
    mqttPublish("Place finger...");
    Serial.printf("Waitng image for fingerId %d...\n", fingerprintId);
    led(LED_WAIT);

    if (!getImage())
    {
        mqttPublish("Error reading finger. Please retry.");
        Serial.println("Error reading finger. Please retry.");
        return false;
    }

    mqttPublish("Remove finger...");
    Serial.println("Got image");
    led(LED_SNAP);

    if (!convertImage(IMAGE_ONE))
    {
        mqttPublish("Error processing finger. Please retry.");
        Serial.println("Error converting image. Please retry.");
        return false;
    }

    delay(1000);

    if (!waitNoFinger())
    {
        mqttPublish("Error waiting no finger. Please retry.");
        Serial.println("Error waiting no finger. Please retry.");
        return false;
    }

    mqttPublish("Place same finger again...");
    Serial.printf("Waiting image for same fingerId %d...\n", fingerprintId);
    led(LED_WAIT);

    if (!getImage())
    {
        mqttPublish("Error reading confirm finger. Please retry.");
        Serial.println("Error reading confirm finger. Please retry.");
        return false;
    }

    mqttPublish("Remove finger...");
    Serial.println("Got image");
    led(LED_SNAP);

    if (!convertImage(IMAGE_TWO))
    {
        mqttPublish("Error processing confirm finger. Please retry.");
        Serial.println("Converting confirm image failed. Please retry.");
        return false;
    }

    if (!saveImage(fingerprintId))
    {
        mqttPublish("Finger mismatch. Please retry.");
        Serial.println("Confirm image failed. Please retry.");
        return false;
    }

    mqttPublish("Fingerprint saved.");
    Serial.println("Fingerprint saved.");
    led(LED_READY);

    return true;
}

bool getImage()
{
    int result;
    int tries = 0;

    do
    {
        result = fingerSensor.getImage();

        if (result == FINGERPRINT_NOFINGER)
        {
            Serial.print(".");
            break;
        }
        else
        {
            Serial.printf("Image scan fail [%d]\n", result);
            break;
        }

    } while ((result != FINGERPRINT_OK) || ++tries < 10);

    return (result == FINGERPRINT_OK);
}

bool convertImage(int slot)
{
    int result = fingerSensor.image2Tz(slot);

    if (result == FINGERPRINT_OK)
    {
        return true;
    }

    Serial.printf("Image convert fail [%d]\n", result);

    return false;
}

bool waitNoFinger()
{
    int result = -1;
    int tries = 0;

    Serial.print("Wait no finger");
    while ((result != FINGERPRINT_NOFINGER) && (++tries < 100))
    {
        Serial.print(".");
        result = fingerSensor.getImage();
    }
    Serial.println(".");

    return (result == FINGERPRINT_NOFINGER);
}

bool saveImage(int fingerprintId)
{
    int result = fingerSensor.createModel();

    if (result != FINGERPRINT_OK)
    {
        Serial.printf("Fingerprint match fail! [%d]", result);

        return false;
    }

    result = fingerSensor.storeModel(fingerprintId);

    if (result != FINGERPRINT_OK)
    {
        Serial.printf("Fingerprint store fail! [%d]", result);
        return false;
    }

    Serial.println("Fingerprint model saved.");

    return true;
}

uint8_t fingerprintDelete()
{
    uint8_t p = -1;
int id = 999;
    p = fingerSensor.deleteModel(id);

    if (p == FINGERPRINT_OK)
    {
        sensorMode = "deleting";
        mqttMessage["mode"] = "deleting";
        mqttMessage["id"] = id;
        mqttMessage["state"] = "Deleted";
        mqttMessage["confidence"] = 0;
        mqttPublish("Deleted");

        led(LED_WRONG);

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
