#include <stdint.h>
typedef unsigned char byte;
#include <Arduino.h>

uint8_t getFingerprintId();
uint8_t getFingerprintEnroll();
uint8_t deleteFingerprint();
void callback(char *topic, byte *payload, unsigned int length);
void ledFinger();
void ledMatch();
void ledWrong();
void ledReady();
void ledWait();
