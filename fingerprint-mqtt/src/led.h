#include <stdint.h>

// Status
#define LED_READY 0

// Normal loop
#define LED_MATCH 2
#define LED_WRONG 3

// Enrolling
#define LED_WAIT 4
#define LED_SNAP 5

void led(uint8_t mode);
