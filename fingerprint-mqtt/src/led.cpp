#include "led.h"
#include "setup.h"

boolean boardLedIsOn = false;
boolean boardLedBlinkMode = false;

void boardLedOn()
{
    digitalWrite(LED_BUILTIN, LOW);
    boardLedIsOn = true;
}

void boardLedOff()
{
    digitalWrite(LED_BUILTIN, HIGH);
    boardLedIsOn = false;
}

void boardLedBlinkLoop()
{
    if (!loopDelay(DELAY_BOARDLED, 250))
    {
        return;
    }

    if (boardLedIsOn)
    {
        boardLedOff();
    }
    else
    {
        boardLedOn();
    }
}

void boardLedLoop()
{
    if (boardLedBlinkMode)
    {
        boardLedBlinkLoop();
    }
    else
    {
        boardLedOn();
    }
}

void boardLedSetBlink()
{
    boardLedBlinkMode = true;
}

void boardLedSetSolid()
{
    boardLedBlinkMode = false;
}

void led(uint8_t mode)
{
    switch (mode)
    {
    case LED_SNAP:
        fingerSensor.LEDcontrol(
            FINGERPRINT_LED_FLASHING,
            50,
            FINGERPRINT_LED_PURPLE,
            1);

        break;

    case LED_MATCH:
        fingerSensor.LEDcontrol(
            FINGERPRINT_LED_BREATHING,
            150,
            FINGERPRINT_LED_BLUE,
            1);
        break;

    case LED_WRONG:
        fingerSensor.LEDcontrol(
            FINGERPRINT_LED_BREATHING,
            30,
            FINGERPRINT_LED_RED,
            2);
        break;

    case LED_READY:
        fingerSensor.LEDcontrol(
            FINGERPRINT_LED_FLASHING,
            15,
            FINGERPRINT_LED_BLUE,
            3);
        break;

    case LED_WAIT:
        fingerSensor.LEDcontrol(
            FINGERPRINT_LED_BREATHING,
            15,
            FINGERPRINT_LED_PURPLE,
            1);
        break;

    default:
        break;
    }
}