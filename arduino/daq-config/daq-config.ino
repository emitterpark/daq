#include <EEPROM.h>

const uint8_t numAn                     = 2;
const uint8_t numDg                     = 2;

struct Conf {
  const uint8_t   lru08[3]              = {2,90,2};
  const uint8_t   anu08[5 * numAn]      = {1,2,1,1,1,1,2,1,1,1};
  const uint16_t  anu16[numAn]          = {1,1};
  const float     anf32[7 * numAn]      = {4,20,4,20,11,13,0,4,20,4,20,11,13,0};
  const uint8_t   dgu08[5 * numDg]      = {1,0,20,1,1,1,0,20,1,1};
  const uint16_t  dgu16[numDg]          = {1,1};
};

Conf conf;

void setup() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.put(0, conf);
}

void loop() {
}
