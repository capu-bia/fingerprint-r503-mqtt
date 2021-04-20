#include "manage.h"
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include "setup.h"
#include "led.h"

#define IMAGE_ONE 1
#define IMAGE_TWO 2

bool processEnroll(uint8_t fingerprintId)
{
    delay(1000);

    mqttPublish("Place finger...");
    Serial.printf("Waiting image for fingerprintId %d...\n", fingerprintId);
    led(LED_WAIT);

    if (!getImage())
    {
        mqttPublish("Error reading finger. Please retry.");
        Serial.println("Error reading finger. Please retry.");
        led(LED_WRONG);
        return false;
    }

    mqttPublish("Remove finger...");
    delay(1000);
    Serial.println("Got image");
    led(LED_SNAP);

    if (!convertImage(IMAGE_ONE))
    {
        mqttPublish("Error processing finger. Please retry.");
        Serial.println("Error converting image. Please retry.");
        led(LED_WRONG);
        return false;
    }

    delay(1000);

    if (!waitNoFinger())
    {
        mqttPublish("Error waiting no finger. Please retry.");
        Serial.println("Error waiting no finger. Please retry.");
        led(LED_WRONG);
        return false;
    }

    mqttPublish("Place same finger again...");
    delay(1000);
    Serial.printf("Waiting image for same fingerId %d...\n", fingerprintId);
    led(LED_WAIT);

    if (!getImage())
    {
        mqttPublish("Error reading confirm finger. Please retry.");
        Serial.println("Error reading confirm finger. Please retry.");
        led(LED_WRONG);
        return false;
    }

    mqttPublish("Remove finger...");
    delay(1000);
    Serial.println("Got image");
    led(LED_SNAP);

    if (!convertImage(IMAGE_TWO))
    {
        mqttPublish("Error processing confirm finger. Please retry.");
        Serial.println("Converting confirm image failed. Please retry.");
        led(LED_WRONG);
        return false;
    }

    if (!saveImage(fingerprintId))
    {
        mqttPublish("Finger mismatch. Please retry.");
        Serial.println("Confirm image failed. Please retry.");
        led(LED_WRONG);
        return false;
    }

    mqttPublish("Fingerprint saved.");
    delay(1000);
    Serial.println("Fingerprint saved.");
    led(LED_READY);

    delay(500);

    return true;
}

bool getImage()
{
    int result;
    uint8_t tries = 0;

    do
    {
        result = fingerSensor.getImage();

        if (result != FINGERPRINT_OK)
        {
            Serial.print(".");
            led(LED_WAIT);
            delay(100);
        }

    } while ((result != FINGERPRINT_OK) && ++tries < 10000);

    return (result == FINGERPRINT_OK);
}

bool convertImage(uint8_t slot)
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
    uint8_t tries = 0;

    Serial.print("Wait no finger");
    while ((result != FINGERPRINT_NOFINGER) && (++tries < 100))
    {
        Serial.print(".");
        result = fingerSensor.getImage();
    }
    Serial.println(".");

    return (result == FINGERPRINT_NOFINGER);
}

bool saveImage(uint8_t fingerprintId)
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

boolean processDelete(uint8_t fingerprintId)
{

    mqttPublish("Deleting fingerprint model...");
    led(LED_WAIT);

    delay(1000);

    int result = fingerSensor.deleteModel(fingerprintId);

    if (result == FINGERPRINT_OK)
    {
        mqttPublish("Fingerprint model deleted.");
        Serial.println("Fingerprint model deleted.");

        led(LED_READY);

        delay(1000);

        return true;
    }

    mqttPublish("Error deleting fingerprint model!");
    Serial.printf("Error deleting fingerprint id %d code [%x]\n\n", fingerprintId, result);

    led(LED_WRONG);

    delay(1000);

    return false;
}
