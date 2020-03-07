#include <EEPROM.h>

const uint8_t numAn                     = 2;
const uint8_t numDg                     = 2;

struct Conf {
  const uint8_t   ge_u08[4]             = {2,90,10,5};
  const uint8_t   an_u08[5 * numAn]     = {1,2,1,1,1,1,2,1,1,1};
  const uint16_t  an_u16[numAn]         = {1,1};
  const float     an_f32[6 * numAn]     = {0.004,0.020,0.004,0.020,0.004,0.020,0.004,0.020,0.004,0.020,0.004,0.020};
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
