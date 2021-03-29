#include <stdint.h>
#include <stdbool.h>

bool processEnroll(int fingerId);

uint8_t processDelete();
uint8_t fingerprintDelete();

bool getImage();
bool saveImage(int fingerprintId);
bool convertImage(int slot);
bool waitNoFinger();
