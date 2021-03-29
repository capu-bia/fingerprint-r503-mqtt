#include <stdint.h>
typedef unsigned char byte;
#include <Arduino.h>

uint8_t fingerprintReading();
void callback(char *topic, byte *payload, unsigned int length);
