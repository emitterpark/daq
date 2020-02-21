#include <avr/wdt.h>
#include <EEPROM.h>
#include <CayenneLPP.h>
#include <Wire.h>
#include <INA226.h>
#include <WebUSB.h>

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

// uint8_t ge_u08[] 
const uint8_t ge_u08_lora_dr      = 0;  
const uint8_t ge_u08_lora_port    = 1;  
const uint8_t ge_u08_poll         = 2;  // input max 240sec
const uint8_t ge_u08_report       = 3;  // input max 240min

// uint8_t an_u08[]
const uint8_t an_u08_enable       = 0;  
const uint8_t an_u08_unit         = 1;
const uint8_t an_u08_within_report= 2;
const uint8_t an_u08_low_report   = 3;
const uint8_t an_u08_high_report  = 4; 
// float an_f32[]
const uint8_t an_f32_in_min       = 0;  
const uint8_t an_f32_in_max       = 1;  
const uint8_t an_f32_out_min      = 2;  
const uint8_t an_f32_out_max      = 3;  
const uint8_t an_f32_low          = 4;  
const uint8_t an_f32_high         = 5;  

// uint8_t dg_u08[]
const uint8_t dg_u08_enable       = 0;  
const uint8_t dg_u08_unit         = 1;  
const uint8_t dg_u08_debounce     = 2;  // input max 250ms
const uint8_t dg_u08_low_report   = 3;
const uint8_t dg_u08_high_report  = 4;

const uint8_t numAn               = 2;
const uint8_t numDg               = 2;

struct Conf {
  uint8_t   ge_u08[4];
  uint8_t   an_u08[5 * numAn];
  float     an_f32[6 * numAn];
  uint8_t   dg_u08[5 * numDg];      
};
Conf conf;

float       an[numAn];
uint8_t     dg[numDg];

const uint8_t WITHIN              = 2;

uint8_t     an_prev[numAn]        = {WITHIN, WITHIN};
uint8_t     dg_prev[numDg]        = {LOW, LOW};

unsigned long tmrPoll, tmrReport, tmrRandom;
volatile bool isReport;
String strUsbSerial, strLoraSerial;
bool loraJoin = false, loraReport = true;

CayenneLPP lpp(51);
INA226 analog;
WebUSB WebUSBSerial(1 /* https:// */, "leanofis-iot.github.io/lora");
#define usbSerial WebUSBSerial
#define loraSerial Serial1

void setup() {
  wdt_enable(WDTO_8S);
  setUsb();
  setPin();  
  loadConf();    
  setAnalog();  
  //delayRandom();  ?? do inloop    
  setLora(); 
  tmrPoll = millis();
  tmrReport = millis();
}
void loop() {
  if (isPollInterval()) {
    readAnalog();    
  }  
  readDigital();  
  if (isReport) {
    report();  
  }
  if (isReportInterval()) {
    report();    
  }  
  readLoraSerial();
  readUsbSerial();
  wdt_reset();
}
void readAnalog() {
  // wire.end();    
  for (uint8_t ch = 0; ch < numAn; ch++) {
    const uint8_t _enable = an_u08_enable + ch * sizeof(conf.an_u08) / numAn; 
    if (conf.an_u08[_enable]) {   
      while (AN_ALR_PIN[ch]);
      analog.begin(0x40 + ch);
      an[ch] = analog.readShuntCurrent();
      if (analog.isAlert());      
      const uint8_t _in_min = an_f32_in_min + ch * sizeof(conf.an_f32) / numAn;
      const uint8_t _in_max = an_f32_in_max + ch * sizeof(conf.an_f32) / numAn;
      const uint8_t _out_min = an_f32_out_min + ch * sizeof(conf.an_f32) / numAn;
      const uint8_t _out_max = an_f32_out_max + ch * sizeof(conf.an_f32) / numAn;
      an[ch] = (an[ch] - conf.an_f32[_in_min]) * (conf.an_f32[_out_max] - conf.an_f32[_out_min]) / (conf.an_f32[_in_max] - conf.an_f32[_in_min]) + conf.an_f32[_out_min];      
      isAnalogReport(ch);      
    }                    
  }    
}
void isAnalogReport(const uint8_t ch) {       
  const uint8_t _low = an_f32_low + ch * sizeof(conf.an_f32) / numAn; 
  const uint8_t _high = an_f32_high + ch * sizeof(conf.an_f32) / numAn;
  const uint8_t _within_report = an_u08_within_report + ch * sizeof(conf.an_u08) / numAn;
  const uint8_t _low_report = an_u08_low_report + ch * sizeof(conf.an_u08) / numAn; 
  const uint8_t _high_report = an_u08_high_report + ch * sizeof(conf.an_u08) / numAn; 
  if (an[ch] > conf.an_f32[_low] && an[ch] < conf.an_f32[_high]) {      
    if (an_prev[ch] != WITHIN) {
      if (conf.an_u08[_within_report]) {
        isReport = true;
      }           
    }
    an_prev[ch] = WITHIN;              
  } else if (an[ch] <= conf.an_f32[_low]) {    
    if (an_prev[ch] != LOW) {
      if (conf.an_u08[_low_report]) {
        isReport = true;
      }                 
    }
    an_prev[ch] = LOW;              
  } else if (an[ch] >= conf.an_f32[_high]) {     
    if (an_prev[ch] != HIGH) {
      if (conf.an_u08[_high_report]) {
        isReport = true;
      }                 
    }
    an_prev[ch] = HIGH;
  }  
}
void readDigital() {
  for (uint8_t ch = 0; ch < numDg; ch++) {
    const uint8_t _enable = dg_u08_enable + ch * sizeof(conf.dg_u08) / numDg;
    if (conf.dg_u08[_enable]) { 
      dg[ch] = digitalRead(DIG_PIN[ch]);
      const uint8_t _debounce = dg_u08_debounce + ch * sizeof(conf.dg_u08) / numDg;
      delay(conf.dg_u08[_debounce]); 
      if (dg[ch] == digitalRead(DIG_PIN[ch])) {
        isDigitalReport(ch);
      } else {
        dg[ch] = digitalRead(DIG_PIN[ch]);           
      }                
    }    
  }
}
void isDigitalReport(const uint8_t ch) {
  const uint8_t _low_report = dg_u08_low_report + ch * sizeof(conf.dg_u08) / numDg; 
  const uint8_t _high_report = dg_u08_high_report + ch * sizeof(conf.dg_u08) / numDg;       
  if (dg[ch] != dg_prev[ch]) {
    if (dg[ch] == LOW) {
      if (conf.an_u08[_low_report]) {
        isReport = true;
      }
      dg_prev[ch] = LOW;  
    } else if (dg[ch] == HIGH) { 
      if (conf.an_u08[_high_report]) {
        isReport = true;
      }
      dg_prev[ch] = HIGH;
    }
  }         
}
bool isPollInterval() {
  const uint8_t _poll = ge_u08_poll;
  if ((millis() - tmrPoll) / 1000 >= conf.ge_u08[_poll]) {
    tmrPoll = millis();
    return true;
  }
}
bool isReportInterval() {
  const uint8_t _report = ge_u16_report;
  if ((millis() - tmrReport) / 60000 >= conf.ge_u16[_report]) {
    tmrReport = millis();
    return true;
  }
}
void readLoraSerial() { 
  while (loraSerial.available()) {
    const char chr = (char)loraSerial.read();    
    strLoraSerial += chr;
    if (chr == '\n') {
      strLoraSerial.trim();
      if (strLoraSerial.endsWith(F("Join Success"))) {        
        // delay
        loraSerial.print(F("at+set_config=lora:dr:")); 
        loraSerial.println(conf.ge_u08[ge_u08_lora_dr]);
      } else if (strLoraSerial.endsWith("DR" + String(conf.ge_u08[ge_u08_lora_dr]) +" success")) { 
        loraJoin = true; 
        digitalWrite(LED_PIN, HIGH);       
      } else if (strLoraSerial.endsWith(F("send success"))) { 
        loraReport = true;
      }
      //if (usbSerial) {
      usbSerial.println(strLoraSerial); 
      //}
      strLoraSerial = "";
    }
  }
}
void readUsbSerial() {
  while (usbSerial && usbSerial.available()) {
    const char chr = (char)usbSerial.read();
    strUsbSerial += chr;
    if (chr == '\n') {
      strUsbSerial.trim();
      const uint8_t num = strUsbSerial.substring(7,9).toInt();
      const int16_t valInt = strUsbSerial.substring(9).toInt();
      const float valFloat = strUsbSerial.substring(9).toFloat();       
      if (strUsbSerial.startsWith(F("at"))) {
        loraSerial.println(strUsbSerial);
      } else if (strUsbSerial.startsWith(F("xge_u08"))) {
        conf.ge_u08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xan_u08"))) {
        conf.an_u08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xan_f32"))) {
        conf.an_f32[num] = valFloat;
      } else if (strUsbSerial.startsWith(F("xsave"))) {
        EEPROM.put(0, conf);
        resetMe(); 
      } else if (strUsbSerial.startsWith(F("xget_ge"))) {
        getGeneral();
      } else if (strUsbSerial.startsWith(F("xget_ch"))) {
        getChannels();
      } else if (strUsbSerial.startsWith(F("xfetch"))) {
        fetchChannels();        
      } else if (strUsbSerial.startsWith(F("xss"))) {
        tm.Second = strUsbSerial.substring(3).toInt();
      } else if (strUsbSerial.startsWith(F("xmm"))) {
        tm.Minute = strUsbSerial.substring(3).toInt();
      } else if (strUsbSerial.startsWith(F("xhh"))) {
        tm.Hour = strUsbSerial.substring(3).toInt();
      } else if (strUsbSerial.startsWith(F("xdd"))) {
        tm.Day = strUsbSerial.substring(3).toInt();
      } else if (strUsbSerial.startsWith(F("xwd"))) {
        tm.Wday = strUsbSerial.substring(3).toInt();
      } else if (strUsbSerial.startsWith(F("xmh"))) {
        tm.Month = strUsbSerial.substring(3).toInt();
      } else if (strUsbSerial.startsWith(F("xyy"))) {
        tm.Year = strUsbSerial.substring(3).toInt() - 1970;
      } else if (strUsbSerial.startsWith(F("xtime"))) {
        RTC.write(tm);
      }
      strUsbSerial = "";
    }
  }   
}
void getGeneral() {
  String str; 
  for (uint8_t i = 0; i < sizeof(conf.ge_u08); i++) {    
    usbSerial.print(F("xge_u08"));
    str = '0' + i;
    usbSerial.print(str.substring(str.length() - 2));
    usbSerial.println(conf.ge_u08[i]);    
  }  
}
void getChannels() { 
  String str;  
  for (uint8_t i = 0; i < sizeof(conf.an_u08); i++) {
    usbSerial.print(F("xan_u08"));
    str = '0' + i;
    usbSerial.print(str.substring(str.length() - 2));
    usbSerial.println(conf.an_u08[i]);    
  }
  for (uint8_t i = 0; i < sizeof(conf.an_f32); i++) {
    usbSerial.print(F("xan_f32"));
    str = '0' + i;
    usbSerial.print(str.substring(str.length() - 2));
    usbSerial.println(conf.an_f32[i]);    
  }
  for (uint8_t i = 0; i < sizeof(conf.dg_u08); i++) {
    usbSerial.print(F("xdg_u08"));
    str = '0' + i;
    usbSerial.print(str.substring(str.length() - 2));
    usbSerial.println(conf.dg_u08[i]);    
  }  
}
void fetchChannels() { 
  String str;
  for (uint8_t i = 0; i < numAn; i++) {
    usbSerial.print(F("xan_val"));
    str = '0' + i;
    usbSerial.print(str.substring(str.length() - 2));
    usbSerial.println(an[i]);    
  }
  for (uint8_t i = 0; i < numDg; i++) {
    usbSerial.print(F("xdg_val"));
    str = '0' + i;
    usbSerial.print(str.substring(str.length() - 2));
    usbSerial.println(dg[i]);    
  }  
}
void loadConf() {
  EEPROM.get(0, conf);  
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
void setAnalog() {
  for (uint8_t ch = 0; ch < numAn; ch++) { 
    // wire.end();   
    analog.begin(0x40 + ch);
    analog.configure(INA226_AVERAGES_128, INA226_BUS_CONV_TIME_140US, INA226_SHUNT_CONV_TIME_8244US, INA226_MODE_SHUNT_CONT);
    analog.calibrate(3.3, 0.30);
    analog.enableConversionReadyAlert();               
  } 
}
void setLora() {
  loraSerial.begin(115200);
  delay(100);
  digitalWrite(LORA_RES_PIN, HIGH);
}
void setUsb() {
  if (USBSTA >> VBUS & 1) {    
    usbSerial.begin(9600);    
    while (!usbSerial) {
      wdt_reset();
    }
    usbSerial.flush();
  } 
}   
void report() {
  wdt_reset();
  isReport = false;  
  if (loraJoin && loraReport) {
    loraReport = false;      
    lpp.reset();  
    for (uint8_t ch = 0; ch < numAn; ch++) {
      const uint8_t _unit = conf.an_u08[an_u08_unit + ch * sizeof(conf.an_u08) / numAn];      
      if (_unit == LPP_ANALOG_INPUT) {
        lpp.addAnalogInput(1 + ch, an[ch]);
      } else if (_unit == LPP_LUMINOSITY) {
        lpp.addLuminosity(1 + ch, an[ch]);
      } else if (_unit == LPP_TEMPERATURE) {
        lpp.addTemperature(1 + ch, an[ch]);
      } else if (_unit == LPP_RELATIVE_HUMIDITY) {
        lpp.addRelativeHumidity(1 + ch, an[ch]);
      } else if (_unit == LPP_BAROMETRIC_PRESSURE) {
        lpp.addBarometricPressure(1 + ch, an[ch]);
      } else if (_unit == LPP_VOLTAGE) {
        lpp.addVoltage(1 + ch, an[ch]);
      } else if (_unit == LPP_CURRENT) {
        lpp.addCurrent(1 + ch, an[ch]);
      } else if (_unit == LPP_PERCENTAGE) {
        lpp.addPercentage(1 + ch, an[ch]);
      } else if (_unit == LPP_ALTITUDE) {
        lpp.addAltitude(1 + ch, an[ch]);
      } else if (_unit == LPP_POWER) {
        lpp.addPower(1 + ch, an[ch]);
      } else if (_unit == LPP_DIRECTION) {
        lpp.addDirection(1 + ch, an[ch]);
      } else if (_unit == LPP_DIGITAL_INPUT) {
        lpp.addDigitalInput(1 + ch, an[ch]);
      } else if (_unit == LPP_SWITCH) {
        lpp.addSwitch(1 + ch, an[ch]);
      } else if (_unit == LPP_PRESENCE) {  
        lpp.addPresence(1 + ch, an[ch]);      
      }
    } 
    for (uint8_t ch = 0; ch < numDg; ch++) {
      const uint8_t _unit = conf.dg_u08[dg_u08_unit + ch * sizeof(conf.dg_u08) / numDg];      
      if (_unit == LPP_DIGITAL_INPUT) {
        lpp.addDigitalInput(3 + ch, dg[ch]);
      } else if (_unit == LPP_SWITCH) {
        lpp.addSwitch(3 + ch, dg[ch]);
      } else if (_unit == LPP_PRESENCE) {
        lpp.addPresence(3 + ch, dg[ch]);
      }
    }
    for (uint8_t ch = 0; ch < numMo; ch++) {
      const uint8_t _unit = conf.mo_u08[mo_u08_unit + ch * sizeof(conf.mo_u08) / numMo];      
      if (_unit == LPP_ANALOG_INPUT) {
        lpp.addAnalogInput(5 + ch, mo[ch]);
      } else if (_unit == LPP_LUMINOSITY) {
        lpp.addLuminosity(5 + ch, mo[ch]);
      } else if (_unit == LPP_TEMPERATURE) {
        lpp.addTemperature(5 + ch, mo[ch]);
      } else if (_unit == LPP_RELATIVE_HUMIDITY) {
        lpp.addRelativeHumidity(5 + ch, mo[ch]);
      } else if (_unit == LPP_BAROMETRIC_PRESSURE) {
        lpp.addBarometricPressure(5 + ch, mo[ch]);
      } else if (_unit == LPP_VOLTAGE) {
        lpp.addVoltage(5 + ch, mo[ch]);
      } else if (_unit == LPP_CURRENT) {
        lpp.addCurrent(5 + ch, mo[ch]);
      } else if (_unit == LPP_PERCENTAGE) {
        lpp.addPercentage(5 + ch, mo[ch]);
      } else if (_unit == LPP_ALTITUDE) {
        lpp.addAltitude(5 + ch, mo[ch]);
      } else if (_unit == LPP_POWER) {
        lpp.addPower(5 + ch, mo[ch]);
      } else if (_unit == LPP_DIRECTION) {
        lpp.addDirection(5 + ch, mo[ch]);
      } else if (_unit == LPP_DIGITAL_INPUT) {
        lpp.addDigitalInput(5 + ch, mo[ch]);
      } else if (_unit == LPP_SWITCH) {
        lpp.addSwitch(5 + ch, mo[ch]);
      } else if (_unit == LPP_PRESENCE) {  
        lpp.addPresence(5 + ch, mo[ch]);      
      }
    } 
    loraSerial.print("at+send=lora:" + String(conf.ge_u08[ge_u08_lora_port]) + ':'); 
    //loraSerial.println(lppGetBuffer());
    loraSerial.println((char*)(lpp.getBuffer()));
  } else {
    resetMe();
  }      
}
void delayRandom() {
  randomSeed(analogRead(RANDOM_PIN));
  const unsigned long rnd = random(24) * 5000;   // max 2 minutes
  tmrRandom = millis();
  while (millis() - tmrRandom < rnd) {
    wdt_reset();    
  }
}
void resetMe() {
  wdt_enable(WDTO_15MS);
  while(true); 
}
/*
String lppGetBuffer() {
  String str;
  for(uint8_t ii = 0; ii < lpp.getSize(); ii++){    
    if (lpp.getBuffer()[ii] < 16) {
      str += '0';       
    }
    str += String(lpp.getBuffer()[ii], HEX);
    str.toUpperCase();        
  }
  return str;
} 
char buf[15];
str.toCharArray(buf, sizeof(buf));
float f = atof(buf);
int32_t i = atol(buf);
// string.toFloat()
// atol() atof()

 */
