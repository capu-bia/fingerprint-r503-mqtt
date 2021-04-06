#include <stdint.h>
#include <stdbool.h>

bool processEnroll(uint8_t fingerprintId);
bool processDelete(uint8_t fingerprintId);

bool getImage();
bool saveImage(uint8_t fingerprintId);
bool convertImage(uint8_t slot);
bool waitNoFinger();
