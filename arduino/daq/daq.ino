// Bootloader: Board SparkFun Pro Micro 3.3V 8MHZ
// sparkfun boards.txt 
// promicro.menu.cpu.8MHzatmega32U4.bootloader.extended_fuses=0xFF
// promicro.build.extra_flags={build.usb_flags} "-DUSB_VERSION=0x210"
// or C:Users/../AppData/Local/Arduino../packages/arduino/hardware/avr/../cores/arduino/USBCore.h 
// line 130: #define USB_VERSION 0x210

#include <avr/wdt.h>
#include <Timer.h>                // https://github.com/JChristensen/Timer
#include <EEPROM.h>
#include <CayenneLPP.h>           // https://github.com/ElectronicCats/CayenneLPP
#include <Wire.h>
#include <INA226.h>               // https://github.com/jarzebski/Arduino-INA226
#include <WebUSB.h>               // https://github.com/webusb/arduino

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

const uint8_t lru08_dr            = 0;  
const uint8_t lru08_port          = 1;  
const uint8_t lru08_report        = 2;

const uint8_t anu08_enable        = 0;  
const uint8_t anu08_unit          = 1; 

const uint8_t anu16_duration      = 0; 

const uint8_t anf32_in_min        = 0;  
const uint8_t anf32_in_max        = 1;  
const uint8_t anf32_out_min       = 2;  
const uint8_t anf32_out_max       = 3;  
const uint8_t anf32_low           = 4;  
const uint8_t anf32_high          = 5;
const uint8_t anf32_calibrate     = 6;  

const uint8_t dgu08_enable        = 0;  
const uint8_t dgu08_unit          = 1;  
const uint8_t dgu08_debounce      = 2;
const uint8_t dgu08_low_report    = 3;
const uint8_t dgu08_high_report   = 4;

const uint8_t dgu16_duration      = 0;

const uint8_t numAn               = 2;
const uint8_t numDg               = 2;

struct Conf {
  uint8_t   lru08[3];
  uint8_t   anu08[2 * numAn];
  uint16_t  anu16[numAn];
  float     anf32[7 * numAn];
  uint8_t   dgu08[5 * numDg];
  uint16_t  dgu16[numDg];      
};
Conf conf;

float       an[numAn];
uint8_t     dg[numDg];

const uint8_t MID                 = 2;

uint8_t     anPrev[numAn]         = {MID, MID};
uint8_t     dgPrev[numDg]         = {LOW, LOW};

int         anDuration[numAn] = {-1, -1};
int         dgDuration[numDg] = {-1, -1};
int         ledOscForever;

bool isLoraJoin, isUsb, isIntervalReport, isLoraSerial;
String strUsbSerial, strLoraSerial;
const float rshunt = 3.9;

Timer t;
CayenneLPP lpp(51);
INA226 analog;
//WebUSB WebUSBSerial(1 /* https:// */, "emitterpark.github.io/daq");
WebUSB WebUSBSerial(1 /* https:// */, "127.0.0.1:5500");
#define usbSerial WebUSBSerial
#define loraSerial Serial1

void setup() {
  setPin();
  loadConf(); 
  setUsbSerial();   
  setAnalog();
  setDigital();  
  setLoraSerial();
  //t.after(tmrRandom(), setLoraSerial);
  printAll();       
  t.every(conf.lru08[lru08_report] * 1000L * 60, intervalReport);
  ledOscForever = t.oscillate(LED_PIN, 500, HIGH);  
}
void loop() {
  readAnalog();
  readDigital();  
  readLoraSerial();
  if (isUsb) {
    readUsbSerial();
  }  
  t.update();
}
void report() {   
  for (uint8_t ch = 0; ch < numAn; ch++) {
    t.stop(anDuration[ch]);
    anDuration[ch] = -1;
  }
  for (uint8_t ch = 0; ch < numDg; ch++) {
    t.stop(dgDuration[ch]);
    dgDuration[ch] = -1;
  }
  printMsg(F("report")); 
  if (!isLoraJoin) {
    if (isLoraSerial) {
      if(isIntervalReport) {
        isIntervalReport = false;
        resetCpu();    
      }
      return;
    }
  }           
  lpp.reset();  
  for (uint8_t ch = 0; ch < numAn; ch++) {
    const uint8_t _unit = conf.anu08[anu08_unit + ch * (sizeof(conf.anu08) / sizeof(conf.anu08[0])) / numAn];      
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
    const uint8_t _unit = conf.dgu08[dgu08_unit + ch * (sizeof(conf.dgu08) / sizeof(conf.dgu08[0])) / numDg];      
    if (_unit == LPP_DIGITAL_INPUT) {
      lpp.addDigitalInput(3 + ch, dg[ch]);
    } else if (_unit == LPP_SWITCH) {
      lpp.addSwitch(3 + ch, dg[ch]);
    } else if (_unit == LPP_PRESENCE) {
      lpp.addPresence(3 + ch, dg[ch]);
    }
  }    
  loraSerial.print("at+send=lora:" + String(conf.lru08[lru08_port]) + ':'); 
  loraSerial.println(lppGetBuffer());       
}
void readLoraSerial() { 
  while (loraSerial.available()) {    
    const char chr = (char)loraSerial.read();    
    strLoraSerial += chr;
    if (chr == '\n') {
      strLoraSerial.trim();      
      if (strLoraSerial.endsWith(F("Join Success"))) {        
        delay(10);         
        loraSerial.print(F("at+set_config=lora:dr:")); 
        loraSerial.println(conf.lru08[lru08_dr]); 
      } else if (strLoraSerial.endsWith(F("JOIN_FAIL 99"))) {
        isLoraJoin = false;
        printMsg(F("xjoinlrwCould not Join to Network"));
      } else if (strLoraSerial.indexOf(F("Join retry")) >= 0) {
        isLoraJoin = false;
        t.stop(ledOscForever);
        ledOscForever = t.oscillate(LED_PIN, 500, HIGH);
        printMsg(F("xjoinlrwJoining to Network .."));
      } else if (strLoraSerial.indexOf(F("NO_NETWORK")) >= 0) {
        isLoraJoin = false;
        printMsg(F("xjoinlrwCould not Join to Network")); 
      } else if (strLoraSerial.indexOf(F("configure DR")) >= 0) {
        isLoraJoin = true;         
        t.stop(ledOscForever);        
        digitalWrite(LED_PIN, LOW);              
        printMsg(F("xjoinlrwJoin Success"));              
      }
      if (isUsb) {
        usbSerial.println(strLoraSerial); 
        usbSerial.flush();
      }           
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
      const uint8_t num = strUsbSerial.substring(6, 8).toInt();
      const uint16_t valInt = strUsbSerial.substring(8).toInt();
      const float valFloat = strUsbSerial.substring(8).toFloat();       
      if (strUsbSerial.startsWith(F("at"))) {
        loraSerial.println(strUsbSerial);      
      } else if (strUsbSerial.startsWith(F("xlru08"))) {
        conf.lru08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xanu08"))) {
        conf.anu08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xanu16"))) {
        conf.anu16[num] = (uint16_t)valInt;
      } else if (strUsbSerial.startsWith(F("xanf32"))) {
        conf.anf32[num] = valFloat;
      } else if (strUsbSerial.startsWith(F("xdgu08"))) {
        conf.dgu08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xdgu16"))) {
        conf.dgu16[num] = (uint16_t)valInt;
      } else if (strUsbSerial.startsWith(F("xsave"))) {
        EEPROM.put(0, conf);
        resetCpu();      
      } else if (strUsbSerial.startsWith(F("xreport"))) {
        report();       
      }
      strUsbSerial = "";
    }
  }   
}
void readAnalog() {      
  for (uint8_t ch = 0; ch < numAn; ch++) {
    const uint8_t _enable = anu08_enable + ch * (sizeof(conf.anu08) / sizeof(conf.anu08[0])) / numAn; 
    if (conf.anu08[_enable]) {   
      if (!digitalRead(AN_ALR_PIN[ch])) {        
        analog.begin(0x40 + ch);
        an[ch] = analog.readShuntVoltage();
        if (analog.isAlert()); 
        while (!digitalRead(AN_ALR_PIN[ch])) {          
        }       
        const uint8_t _in_min = anf32_in_min + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn;
        const uint8_t _in_max = anf32_in_max + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn;
        const uint8_t _out_min = anf32_out_min + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn;
        const uint8_t _out_max = anf32_out_max + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn;
        const uint8_t _calibrate = anf32_calibrate + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn;
        an[ch] *= 1000L;
        an[ch] /= rshunt;
        an[ch] = (an[ch] - conf.anf32[_in_min]) * (conf.anf32[_out_max] - conf.anf32[_out_min]) / (conf.anf32[_in_max] - conf.anf32[_in_min]) + conf.anf32[_out_min];      
        //an[ch] *= (1.0 + conf.anf32[_calibrate]);
        //an[ch] += 0.005;        
        //an[ch] = round(an[ch]*100.0)/100.0;
        fetchAnalog(ch);
        isAnalogReport(ch);
      }      
    }                    
  }    
}
void fetchAnalog(const uint8_t ch) { 
  if (!isUsb) {
    return;
  }
  String str;  
  usbSerial.print(F("xanval"));    
  str = '0' + String(ch);
  usbSerial.print(str.substring(str.length() - 2));    
  usbSerial.println(an[ch], 2);
  usbSerial.flush();      
}
void isAnalogReport(const uint8_t ch) {       
  const uint8_t _low = anf32_low + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn; 
  const uint8_t _high = anf32_high + ch * (sizeof(conf.anf32) / sizeof(conf.anf32[0])) / numAn;  
  const uint8_t _duration = anu16_duration + ch * (sizeof(conf.anu16) / sizeof(conf.anu16[0])) / numAn;    
  if (an[ch] > conf.anf32[_low] && an[ch] < conf.anf32[_high]) {    
    if (anPrev[ch] != MID) {
      anPrev[ch] = MID;                 
      if (anDuration[ch] > 0) {
        t.stop(anDuration[ch]);
        anDuration[ch] = -1;
        return;
      }              
      t.stop(anDuration[ch]);
      anDuration[ch] = t.after(conf.anu16[_duration] * 1000L, report);                 
    }                  
  } else if (an[ch] <= conf.anf32[_low]) {    
    if (anPrev[ch] != LOW) {
      anPrev[ch] = LOW;                      
      if (anDuration[ch] > 0) {
        t.stop(anDuration[ch]);
        anDuration[ch] = -1;
        return; 
      }      
      t.stop(anDuration[ch]);
      anDuration[ch] = t.after(conf.anu16[_duration] * 1000L, report);
    }                  
  } else if (an[ch] >= conf.anf32[_high]) {     
    if (anPrev[ch] != HIGH) {
      anPrev[ch] = HIGH;                 
      if (anDuration[ch] > 0) {
        t.stop(anDuration[ch]);
        anDuration[ch] = -1;
        return;
      }      
      t.stop(anDuration[ch]);
      anDuration[ch] = t.after(conf.anu16[_duration] * 1000L, report);
    }    
  }  
}
void readDigital() {
  for (uint8_t ch = 0; ch < numDg; ch++) {  
    const uint8_t _enable = dgu08_enable + ch * (sizeof(conf.dgu08) / sizeof(conf.dgu08[0])) / numDg;
    if (conf.dgu08[_enable]) {
      dg[ch] = !digitalRead(DIG_PIN[ch]);
      if (dg[ch] != dgPrev[ch]) {        
        const uint8_t _debounce = dgu08_debounce + ch * (sizeof(conf.dgu08) / sizeof(conf.dgu08[0])) / numDg;        
        delay(conf.dgu08[_debounce]);     
        if (dg[ch] == !digitalRead(DIG_PIN[ch])) {
          isDigitalReport(ch);
        } else {
          dg[ch] = !digitalRead(DIG_PIN[ch]);           
        }
      }                       
    }    
  }
}
void fetchDigital(const uint8_t ch) { 
  if (!isUsb) {
    return;
  }
  String str;
  usbSerial.print(F("xdgval"));    
  str = '0' + String(ch);
  usbSerial.print(str.substring(str.length() - 2));    
  usbSerial.println(dg[ch]);
  usbSerial.flush();       
}
void isDigitalReport(const uint8_t ch) {
  const uint8_t _low_report = dgu08_low_report + ch * (sizeof(conf.dgu08) / sizeof(conf.dgu08[0])) / numDg; 
  const uint8_t _high_report = dgu08_high_report + ch * (sizeof(conf.dgu08) / sizeof(conf.dgu08[0])) / numDg;
  const uint8_t _duration = dgu16_duration + ch * (sizeof(conf.dgu16) / sizeof(conf.dgu16[0])) / numDg;       
  if (dg[ch] != dgPrev[ch]) {
    fetchDigital(ch);
    if (dg[ch] == LOW) {
      dgPrev[ch] = LOW;                            
      if (dgDuration[ch] > 0) {
        t.stop(dgDuration[ch]);
        dgDuration[ch] = -1;        
        return; 
      }       
      if (conf.dgu08[_low_report]) {                
        t.stop(dgDuration[ch]);        
        dgDuration[ch] = t.after(conf.dgu16[_duration] * 1000L, report);                               
      }          
    } else if (dg[ch] == HIGH) { 
      dgPrev[ch] = HIGH;                       
      if (dgDuration[ch] > 0) {
        t.stop(dgDuration[ch]);
        dgDuration[ch] = -1;        
        return;
      }            
      if (conf.dgu08[_high_report]) {                
        t.stop(dgDuration[ch]);        
        dgDuration[ch] = t.after(conf.dgu16[_duration] * 1000L, report);                                  
      }       
    }
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
    analog.begin(0x40 + ch);
    analog.configure(INA226_AVERAGES_1024, INA226_BUS_CONV_TIME_140US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_CONT);
    analog.calibrate(3.9, 0.020);
    analog.readShuntVoltage();
    analog.enableConversionReadyAlert(); 
    if (analog.isAlert());              
  } 
}
void setDigital() {
  for (uint8_t ch = 0; ch < numDg; ch++) {
    dg[ch] = !digitalRead(DIG_PIN[ch]);
    dgPrev[ch] = dg[ch];    
  }  
}
void setLoraSerial() {
  while (!loraSerial) {    
  }
  loraSerial.begin(115200);    
  delay(10);
  pinMode(LORA_RES_PIN, INPUT);
  delay(1000);
  isLoraSerial = true;
  loraSerial.println("at+version");    
}
void setUsbSerial() {
  if (USBSTA >> VBUS & 1) {          
    while (!usbSerial) {      
    }
  }  
  usbSerial.begin(9600);  
  usbSerial.flush(); 
  isUsb = true;   
}
void printAll() {
  printChFetch();
  printMsg(F("xdeviceeLoraWAN Wireless DAQ"));   
  printMsg(F("xversionFirmware 1.0.1"));
  printChConf();
  printLoraConf();
  printLoraKeys();
  printMsg(F("xjoinlrwJoining to Network .."));
}
void printMsg(String msg) {
  if (!isUsb) {
    return;
  }
  usbSerial.println(msg);
  usbSerial.flush();
}
void printChFetch() {
  if (!isUsb) {
    return;
  }
  for (uint8_t ch = 0; ch < numAn; ch++) {
    fetchAnalog(ch);       
  }
  for (uint8_t ch = 0; ch < numDg; ch++) {
    fetchDigital(ch);       
  }
}
void printChConf() {
  if (!isUsb) {
    return;
  }
  String str;
  for (uint8_t i = 0; i < (sizeof(conf.anu08) / sizeof(conf.anu08[0])); i++) {
    usbSerial.print(F("xanu08"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.anu08[i]);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < (sizeof(conf.anu16) / sizeof(conf.anu16[0])); i++) {
    usbSerial.print(F("xanu16"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.anu16[i]);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < (sizeof(conf.anf32) / sizeof(conf.anf32[0])); i++) {
    usbSerial.print(F("xanf32"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.anf32[i], 2); 
    usbSerial.flush();   
  }
  for (uint8_t i = 0; i < (sizeof(conf.dgu08) / sizeof(conf.dgu08[0])); i++) {
    usbSerial.print(F("xdgu08"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.dgu08[i]);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < (sizeof(conf.dgu16) / sizeof(conf.dgu16[0])); i++) {
    usbSerial.print(F("xdgu16"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.dgu16[i]);
    usbSerial.flush();    
  }    
}
void printLoraConf() {
  if (!isUsb) {
    return;
  }
  String str;
  for (uint8_t i = 0; i < sizeof(conf.lru08); i++) {    
    usbSerial.print(F("xlru08"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.lru08[i]);
    usbSerial.flush();    
  } 
}
void printLoraKeys() {
  if (!isUsb) {
    return;
  }
  delay(10);         
  loraSerial.println(F("at+get_config=lora:status")); 
}
void intervalReport() {
 isIntervalReport = true;
 report(); 
}
unsigned long tmrRandom() {
  randomSeed(analogRead(RANDOM_PIN));
  return random(24) * 5000L + 10000L;   // min 10sec, max 2min + 10sec   
}
void resetCpu() {
  wdt_enable(WDTO_15MS);
  while(true);  
}
String lppGetBuffer() {
  String str;
  for(uint8_t i = 0; i < lpp.getSize(); i++){    
    if (lpp.getBuffer()[i] < 16) {
      str += '0';       
    }
    str += String(lpp.getBuffer()[i], HEX);
    str.toUpperCase();        
  }
  return str;
}
