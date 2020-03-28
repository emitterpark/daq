#include <EEPROM.h>

const uint8_t numAn                     = 2;
const uint8_t numDg                     = 2;

struct Conf {
  const uint8_t   ge_u08[3]             = {2,90,5};
  const uint8_t   an_u08[5 * numAn]     = {1,2,1,1,1,1,2,1,1,1};
  const uint16_t  an_u16[numAn]         = {1,1};
  const float     an_f32[7 * numAn]     = {4,20,4,20,11,13,0,4,20,4,20,11,13,0};
  const uint8_t   dg_u08[5 * numDg]     = {1,0,20,1,1,1,0,20,1,1};
  const uint16_t  dg_u16[numDg]         = {1,1};
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
