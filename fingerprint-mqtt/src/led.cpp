#include "led.h"
#include "setup.h"

void led(int mode)
{
    switch (mode)
    {
    case LED_SNAP:
        fingerSensor.led_control(
            FINGERPRINT_LED_FLASHING,
            100,
            FINGERPRINT_LED_PURPLE,
            1);

        break;

    case LED_MATCH:
        fingerSensor.led_control(
            FINGERPRINT_LED_BREATHING,
            150,
            FINGERPRINT_LED_BLUE,
            1);
        break;

    case LED_WRONG:
        fingerSensor.led_control(
            FINGERPRINT_LED_BREATHING,
            30,
            FINGERPRINT_LED_RED,
            2);
        break;

    case LED_READY:
        fingerSensor.led_control(
            FINGERPRINT_LED_FLASHING,
            15,
            FINGERPRINT_LED_BLUE,
            3);
        break;

    case LED_WAIT:
        fingerSensor.led_control(
            FINGERPRINT_LED_BREATHING,
            15,
            FINGERPRINT_LED_PURPLE,
            1);
        break;

    default:
        break;
    }
}