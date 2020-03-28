/*
    INA226 Bi-directional Current/Power Monitor. Simple Example.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/cyfrowy-czujnik-pradu-mocy-ina226.html
    GIT: https://github.com/jarzebski/Arduino-INA226
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <INA226.h>

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

float rs = 3.3;

INA226 ina;

void checkConfig()
{

  for (uint8_t ch = 0; ch < 2; ch++) {        
    ina.begin(0x40 + ch);
  
    Serial.print("Mode:                  ");
    switch (ina.getMode())
    {
      case INA226_MODE_POWER_DOWN:      Serial.println("Power-Down"); break;
      case INA226_MODE_SHUNT_TRIG:      Serial.println("Shunt Voltage, Triggered"); break;
      case INA226_MODE_BUS_TRIG:        Serial.println("Bus Voltage, Triggered"); break;
      case INA226_MODE_SHUNT_BUS_TRIG:  Serial.println("Shunt and Bus, Triggered"); break;
      case INA226_MODE_ADC_OFF:         Serial.println("ADC Off"); break;
      case INA226_MODE_SHUNT_CONT:      Serial.println("Shunt Voltage, Continuous"); break;
      case INA226_MODE_BUS_CONT:        Serial.println("Bus Voltage, Continuous"); break;
      case INA226_MODE_SHUNT_BUS_CONT:  Serial.println("Shunt and Bus, Continuous"); break;
      default: Serial.println("unknown");
    }
    
    Serial.print("Samples average:       ");
    switch (ina.getAverages())
    {
      case INA226_AVERAGES_1:           Serial.println("1 sample"); break;
      case INA226_AVERAGES_4:           Serial.println("4 samples"); break;
      case INA226_AVERAGES_16:          Serial.println("16 samples"); break;
      case INA226_AVERAGES_64:          Serial.println("64 samples"); break;
      case INA226_AVERAGES_128:         Serial.println("128 samples"); break;
      case INA226_AVERAGES_256:         Serial.println("256 samples"); break;
      case INA226_AVERAGES_512:         Serial.println("512 samples"); break;
      case INA226_AVERAGES_1024:        Serial.println("1024 samples"); break;
      default: Serial.println("unknown");
    }
  
    Serial.print("Bus conversion time:   ");
    switch (ina.getBusConversionTime())
    {
      case INA226_BUS_CONV_TIME_140US:  Serial.println("140uS"); break;
      case INA226_BUS_CONV_TIME_204US:  Serial.println("204uS"); break;
      case INA226_BUS_CONV_TIME_332US:  Serial.println("332uS"); break;
      case INA226_BUS_CONV_TIME_588US:  Serial.println("558uS"); break;
      case INA226_BUS_CONV_TIME_1100US: Serial.println("1.100ms"); break;
      case INA226_BUS_CONV_TIME_2116US: Serial.println("2.116ms"); break;
      case INA226_BUS_CONV_TIME_4156US: Serial.println("4.156ms"); break;
      case INA226_BUS_CONV_TIME_8244US: Serial.println("8.244ms"); break;
      default: Serial.println("unknown");
    }
  
    Serial.print("Shunt conversion time: ");
    switch (ina.getShuntConversionTime())
    {
      case INA226_SHUNT_CONV_TIME_140US:  Serial.println("140uS"); break;
      case INA226_SHUNT_CONV_TIME_204US:  Serial.println("204uS"); break;
      case INA226_SHUNT_CONV_TIME_332US:  Serial.println("332uS"); break;
      case INA226_SHUNT_CONV_TIME_588US:  Serial.println("558uS"); break;
      case INA226_SHUNT_CONV_TIME_1100US: Serial.println("1.100ms"); break;
      case INA226_SHUNT_CONV_TIME_2116US: Serial.println("2.116ms"); break;
      case INA226_SHUNT_CONV_TIME_4156US: Serial.println("4.156ms"); break;
      case INA226_SHUNT_CONV_TIME_8244US: Serial.println("8.244ms"); break;
      default: Serial.println("unknown");
    }
    
    Serial.print("Max possible current:  ");
    Serial.print(ina.getMaxPossibleCurrent(), 8);
    Serial.println(" A");
    
    Serial.print("Max current:           ");
    Serial.print(ina.getMaxCurrent(), 8);
    Serial.println(" A");
    
    Serial.print("Max shunt voltage:     ");
    Serial.print(ina.getMaxShuntVoltage(), 8);
    Serial.println(" V");
    
    Serial.print("Max power:             ");
    Serial.print(ina.getMaxPower(), 8);
    Serial.println(" W");

  }
}

void setup() 
{
  setPin();
  Serial.begin(115200);
  while (!Serial);  ;

  Serial.println("Initialize INA226");
  Serial.println("-----------------------------------------------");

  for (uint8_t ch = 0; ch < 2; ch++) {  
    ina.begin(0x40 + ch); 
    //ina.configure(INA226_AVERAGES_1024, INA226_BUS_CONV_TIME_140US, INA226_SHUNT_CONV_TIME_8244US, INA226_MODE_SHUNT_CONT); 
    ina.configure(INA226_AVERAGES_1024, INA226_BUS_CONV_TIME_140US, INA226_SHUNT_CONV_TIME_8244US, INA226_MODE_SHUNT_CONT);  
    ina.calibrate(3.3, 0.020);
    ina.enableConversionReadyAlert(); 
    //ina.setAlertLatch(true);
    if (ina.isAlert());         
  }  

  // Display configuration
  checkConfig();

  Serial.println("-----------------------------------------------");
}

void loop()
{
  for (uint8_t ch = 0; ch < 2; ch++) { 
    while (digitalRead(AN_ALR_PIN[ch])) {
        //wdt_reset();
    } 
    ina.begin(0x40 + ch);
    
    Serial.print("Bus voltage:   ");
    Serial.print(ina.readBusVoltage(), 8);
    Serial.println(" V");
  
    Serial.print("Bus power:     ");
    Serial.print(ina.readBusPower(), 8);
    Serial.println(" W");
  
    /*
    Serial.print("Shunt voltage: ");
    Serial.print(ina.readShuntVoltage(), 8);
    Serial.println(" V");
  
    Serial.print("Shunt current: ");
    Serial.print(ina.readShuntCurrent(), 8);
    Serial.println(" A");
    */
    // ch1 0.0065658333
    // ch2 0.0050508333
    Serial.print("Shunt current: ");
    Serial.print(ina.readShuntVoltage() * (1 + 0.0050508333) / rs, 8);
    Serial.println(" A");
    if (ina.isAlert());

  }
  Serial.println("");
  //delay(10000);
  
}
void setPin() {
  for (uint8_t ch = 0; ch < 2; ch++) {
    pinMode(AN_ALR_PIN[ch], INPUT);
    pinMode(DIG_PIN[ch], INPUT);
  }
  pinMode(LORA_RES_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LORA_RES_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);  
}
