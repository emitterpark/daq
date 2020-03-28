// Bootloader: Board SparkFun Pro Micro 3.3V 8MHZ
// C:Users/../AppData/Local/Arduino../packages/arduino/hardware/avr/../cores/arduino/USBCore.h 
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

// uint8_t ge_u08[] 
const uint8_t ge_u08_lora_dr      = 0;  
const uint8_t ge_u08_lora_port    = 1;  
const uint8_t ge_u08_report       = 2;

// uint8_t an_u08[]
const uint8_t an_u08_enable       = 0;  
const uint8_t an_u08_unit         = 1;
const uint8_t an_u08_low_report   = 2;
const uint8_t an_u08_mid_report   = 3;
const uint8_t an_u08_high_report  = 4; 
// uint16_t an_u16[]
const uint8_t an_u16_duration     = 0; 
// float an_f32[]
const uint8_t an_f32_in_min       = 0;  
const uint8_t an_f32_in_max       = 1;  
const uint8_t an_f32_out_min      = 2;  
const uint8_t an_f32_out_max      = 3;  
const uint8_t an_f32_low          = 4;  
const uint8_t an_f32_high         = 5;
const uint8_t an_f32_calibrate    = 6;
  

// uint8_t dg_u08[]
const uint8_t dg_u08_enable       = 0;  
const uint8_t dg_u08_unit         = 1;  
const uint8_t dg_u08_debounce     = 2;
const uint8_t dg_u08_low_report   = 3;
const uint8_t dg_u08_high_report  = 4;
// uint16_t dg_u16[]
const uint8_t dg_u16_duration     = 0;

const uint8_t numAn               = 2;
const uint8_t numDg               = 2;

struct Conf {
  uint8_t   ge_u08[3];
  uint8_t   an_u08[5 * numAn];
  uint16_t  an_u16[numAn];
  float     an_f32[7 * numAn];
  uint8_t   dg_u08[5 * numDg];
  uint16_t  dg_u16[numDg];      
};
Conf conf;

float       an[numAn];
uint8_t     dg[numDg];

const uint8_t MID                 = 2;

uint8_t     an_prev[numAn]        = {MID, MID};
uint8_t     dg_prev[numDg]        = {LOW, LOW};

int         anDuration[numAn] = {-1, -1};
int         dgDuration[numDg] = {-1, -1};
int         ledOscForever;

bool isLoraJoin, isLoraBusy;
String strUsbSerial, strLoraSerial;
const uint8_t floatToPrint = 6;
const float rshunt = 3.3;

Timer t;
CayenneLPP lpp(51);
INA226 analog;
//WebUSB WebUSBSerial(1 /* https:// */, "leanofis-iot.github.io/daq");
WebUSB WebUSBSerial(1 /* https:// */, "127.0.0.1:5500");
#define usbSerial WebUSBSerial
#define loraSerial Serial1

void setup() {
  //wdt_enable(WDTO_8S);  
  setPin();
  loadConf();    
  setAnalog();    
  setUsbSerial();
  setLoraSerial();    // t.after(tmrRandom(), setLoraSerial);   
  ////t.every(conf.ge_u08[ge_u08_poll] * 1000L, readAnalog);
  //t.every(conf.ge_u08[ge_u08_report] * 1000L * 60, report);
  //ledOscForever = t.oscillate(LED_PIN, 500, HIGH);  
}
void loop() {
  //readAnalog();
  readDigital();  
  //readLoraSerial();
  //readUsbSerial();
  //wdt_reset();
  t.update();
}
void readAnalog() {
  // wire.end();    
  for (uint8_t ch = 0; ch < numAn; ch++) {
    const uint8_t _enable = an_u08_enable + ch * (sizeof(conf.an_u08) / sizeof(conf.an_u08[0])) / numAn; 
    if (conf.an_u08[_enable]) {   
      if (!digitalRead(AN_ALR_PIN[ch])) {        
        analog.begin(0x40 + ch);
        an[ch] = analog.readShuntVoltage();
        if (analog.isAlert()); 
        while (!digitalRead(AN_ALR_PIN[ch])) {
          //wdt_reset();
        }       
        const uint8_t _in_min = an_f32_in_min + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn;
        const uint8_t _in_max = an_f32_in_max + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn;
        const uint8_t _out_min = an_f32_out_min + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn;
        const uint8_t _out_max = an_f32_out_max + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn;
        const uint8_t _calibrate = an_f32_calibrate + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn;
        an[ch] /= rshunt;
        an[ch] *= (1 + conf.an_f32[_calibrate]);
        usbSerial.println(an[ch], 8);
        usbSerial.flush();            
        //an[ch] = (an[ch] - conf.an_f32[_in_min]) * (conf.an_f32[_out_max] - conf.an_f32[_out_min]) / (conf.an_f32[_in_max] - conf.an_f32[_in_min]) + conf.an_f32[_out_min];      
        //isAnalogReport(ch);
      }      
    }                    
  }    
}
void isAnalogReport(const uint8_t ch) {       
  const uint8_t _low = an_f32_low + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn; 
  const uint8_t _high = an_f32_high + ch * (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])) / numAn;
  const uint8_t _low_report = an_u08_low_report + ch * (sizeof(conf.an_u08) / sizeof(conf.an_u08[0])) / numAn;
  const uint8_t _mid_report = an_u08_mid_report + ch * (sizeof(conf.an_u08) / sizeof(conf.an_u08[0])) / numAn; 
  const uint8_t _high_report = an_u08_high_report + ch * (sizeof(conf.an_u08) / sizeof(conf.an_u08[0])) / numAn; 
  const uint8_t _duration = an_u16_duration + ch * (sizeof(conf.an_u16) / sizeof(conf.an_u16[0])) / numAn;    
  if (an[ch] > conf.an_f32[_low] && an[ch] < conf.an_f32[_high]) {    
    if (an_prev[ch] != MID) {
      an_prev[ch] = MID;      
      if (anDuration[ch]) {
        t.stop(anDuration[ch]);
      } else if (conf.an_u08[_mid_report]) {
        // t.stop(anDuration[ch]);
        anDuration[ch] = t.after(conf.an_u16[_duration] * 1000L, report);          
      }         
    }                  
  } else if (an[ch] <= conf.an_f32[_low]) {    
    if (an_prev[ch] != LOW) {
      an_prev[ch] = LOW;
      if (anDuration[ch]) {
        t.stop(anDuration[ch]);
      } else if (conf.an_u08[_low_report]) {
        // t.stop(anDuration[ch]);
        anDuration[ch] = t.after(conf.an_u16[_duration] * 1000L, report);          
      }                  
    }                  
  } else if (an[ch] >= conf.an_f32[_high]) {     
    if (an_prev[ch] != HIGH) {
      an_prev[ch] = HIGH;
      if (anDuration[ch]) {
        t.stop(anDuration[ch]);
      } else if (conf.an_u08[_high_report]) {
        // t.stop(anDuration[ch]);
        anDuration[ch] = t.after(conf.an_u16[_duration] * 1000L, report);          
      }                   
    }    
  }  
}
void readDigital() {
  for (uint8_t ch = 0; ch < numDg; ch++) {  
    const uint8_t _enable = dg_u08_enable + ch * (sizeof(conf.dg_u08) / sizeof(conf.dg_u08[0])) / numDg;
    if (conf.dg_u08[_enable]) { 
      dg[ch] = digitalRead(DIG_PIN[ch]);
      const uint8_t _debounce = dg_u08_debounce + ch * (sizeof(conf.dg_u08) / sizeof(conf.dg_u08[0])) / numDg;
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
  const uint8_t _low_report = dg_u08_low_report + ch * (sizeof(conf.dg_u08) / sizeof(conf.dg_u08[0])) / numDg; 
  const uint8_t _high_report = dg_u08_high_report + ch * (sizeof(conf.dg_u08) / sizeof(conf.dg_u08[0])) / numDg;
  const uint8_t _duration = dg_u16_duration + ch * (sizeof(conf.dg_u16) / sizeof(conf.dg_u16[0])) / numDg;       
  if (dg[ch] != dg_prev[ch]) {
    if (dg[ch] == LOW) {
      dg_prev[ch] = LOW; 
      usbSerial.println("low"); 
      usbSerial.flush();                
      if (dgDuration[ch] == 0) {
        t.stop(dgDuration[ch]);
        dgDuration[ch] = -1;
        return; 
      } 
      if (conf.dg_u08[_low_report]) {        
        t.stop(dgDuration[ch]);
        dgDuration[ch] = t.after(conf.dg_u16[_duration] * 3 * 1000L, report);                        
      }          
    } else if (dg[ch] == HIGH) { 
      dg_prev[ch] = HIGH;
      usbSerial.println("high"); 
      usbSerial.flush();           
      if (dgDuration[ch] == 0) {
        t.stop(dgDuration[ch]);
        dgDuration[ch] = -1;
        return;
      }        
      if (conf.dg_u08[_high_report]) {        
        t.stop(dgDuration[ch]);
        dgDuration[ch] = t.after(conf.dg_u16[_duration] * 3 * 1000L, report);                          
      }       
    }
  }         
}
void readLoraSerial() { 
  while (loraSerial.available()) {
    //wdt_reset();
    const char chr = (char)loraSerial.read();    
    strLoraSerial += chr;
    if (chr == '\n') {
      strLoraSerial.trim();
      
      if (strLoraSerial.endsWith(F("Join Success"))) {        
        // delay

        isLoraJoin = true; 
        //t.stop(ledOscForever);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
     /*   
        loraSerial.print(F("at+set_config=lora:dr:")); 
        loraSerial.println(conf.ge_u08[ge_u08_lora_dr]);
      } else if (strLoraSerial.endsWith("DR" + String(conf.ge_u08[ge_u08_lora_dr]) +" success")) { 
        isLoraJoin = true; 
        t.stop(ledOscForever);
        digitalWrite(LED_PIN, LOW);  
      */     
      } else if (strLoraSerial.endsWith(F("send success"))) { 
        isLoraBusy = false;        
      } 
      
      usbSerial.println(strLoraSerial); 
      usbSerial.flush();     
      strLoraSerial = "";
    }
  }
}
void readUsbSerial() {
  while (usbSerial && usbSerial.available()) {
    //wdt_reset();
    const char chr = (char)usbSerial.read();
    strUsbSerial += chr;
    if (chr == '\n') {
      strUsbSerial.trim();
      const uint8_t num = strUsbSerial.substring(7,9).toInt();
      const uint16_t valInt = strUsbSerial.substring(9).toInt();
      const float valFloat = strUsbSerial.substring(9).toFloat();       
      if (strUsbSerial.startsWith(F("at"))) {
        loraSerial.println(strUsbSerial);      
      } else if (strUsbSerial.startsWith(F("xge_u08"))) {
        conf.ge_u08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xan_u08"))) {
        conf.an_u08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xan_u16"))) {
        conf.an_u16[num] = (uint16_t)valInt;
      } else if (strUsbSerial.startsWith(F("xan_f32"))) {
        conf.an_f32[num] = valFloat;
      } else if (strUsbSerial.startsWith(F("xdg_u08"))) {
        conf.dg_u08[num] = (uint8_t)valInt;
      } else if (strUsbSerial.startsWith(F("xdg_u16"))) {
        conf.dg_u16[num] = (uint16_t)valInt;
      } else if (strUsbSerial.startsWith(F("xsave"))) {
        EEPROM.put(0, conf);
        resetMe();      
      } else if (strUsbSerial.startsWith(F("xget_ge"))) {
        getGeneral();
      } else if (strUsbSerial.startsWith(F("xget_ch"))) {
        getChannels();
      } else if (strUsbSerial.startsWith(F("xfetch"))) {
        fetchChannels();       
      }
      strUsbSerial = "";
    }
  }   
}
void getGeneral() {
  String str; 
  for (uint8_t i = 0; i < sizeof(conf.ge_u08); i++) {    
    usbSerial.print(F("xge_u08"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.ge_u08[i]);
    usbSerial.flush();    
  }  
}
void getChannels() { 
  String str;  
  for (uint8_t i = 0; i < (sizeof(conf.an_u08) / sizeof(conf.an_u08[0])); i++) {
    usbSerial.print(F("xan_u08"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.an_u08[i]);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < (sizeof(conf.an_u16) / sizeof(conf.an_u16[0])); i++) {
    usbSerial.print(F("xan_u16"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.an_u16[i]);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < (sizeof(conf.an_f32) / sizeof(conf.an_f32[0])); i++) {
    usbSerial.print(F("xan_f32"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.an_f32[i], floatToPrint); 
    usbSerial.flush();   
  }
  for (uint8_t i = 0; i < (sizeof(conf.dg_u08) / sizeof(conf.dg_u08[0])); i++) {
    usbSerial.print(F("xdg_u08"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.dg_u08[i]);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < (sizeof(conf.dg_u16) / sizeof(conf.dg_u16[0])); i++) {
    usbSerial.print(F("xdg_u16"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(conf.dg_u16[i]);
    usbSerial.flush();    
  }    
}
void fetchChannels() { 
  String str;
  for (uint8_t i = 0; i < numAn; i++) {
    usbSerial.print(F("xan_val"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(an[i], floatToPrint);
    usbSerial.flush();    
  }
  for (uint8_t i = 0; i < numDg; i++) {
    usbSerial.print(F("xdg_val"));    
    str = '0' + String(i);
    usbSerial.print(str.substring(str.length() - 2));    
    usbSerial.println(dg[i]);
    usbSerial.flush();    
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
    analog.configure(INA226_AVERAGES_1024, INA226_BUS_CONV_TIME_140US, INA226_SHUNT_CONV_TIME_8244US, INA226_MODE_SHUNT_CONT);
    analog.calibrate(3.3, 0.020);
    analog.enableConversionReadyAlert(); 
    if (analog.isAlert());              
  } 
}
void setLoraSerial() {
  while (!loraSerial) {
    //wdt_reset();
  }
  loraSerial.begin(115200);    
  delay(100);
  digitalWrite(LORA_RES_PIN, HIGH);  
  delay(1000);
  loraSerial.println("at+version");
  loraSerial.flush();  
}
void setUsbSerial() {
  if (USBSTA >> VBUS & 1) {          
    while (!usbSerial) {
      //wdt_reset();
    }
  }  
  usbSerial.begin(9600);
  //usbSerial.write("Sketch begins.\r\n");
  usbSerial.flush();    
}
void report() {
  for (uint8_t ch = 0; ch < numDg; ch++) {
    t.stop(dgDuration[ch]);
    dgDuration[ch] = -1;
  }
  usbSerial.println("alarm");
  usbSerial.flush();     
}
/*
void report() {
  //wdt_reset();    
  if (isLoraJoin && (!isLoraBusy)) {
    isLoraBusy = true;      
    lpp.reset();  
    for (uint8_t ch = 0; ch < numAn; ch++) {
      const uint8_t _unit = conf.an_u08[an_u08_unit + ch * (sizeof(conf.an_u08) / sizeof(conf.an_u08[0])) / numAn];      
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
      const uint8_t _unit = conf.dg_u08[dg_u08_unit + ch * (sizeof(conf.dg_u08) / sizeof(conf.dg_u08[0])) / numDg];      
      if (_unit == LPP_DIGITAL_INPUT) {
        lpp.addDigitalInput(3 + ch, dg[ch]);
      } else if (_unit == LPP_SWITCH) {
        lpp.addSwitch(3 + ch, dg[ch]);
      } else if (_unit == LPP_PRESENCE) {
        lpp.addPresence(3 + ch, dg[ch]);
      }
    }    
    loraSerial.print("at+send=lora:" + String(conf.ge_u08[ge_u08_lora_port]) + ':'); 
    //loraSerial.println(lppGetBuffer());
    loraSerial.println((char*)(lpp.getBuffer()));
    t.oscillate(LED_PIN, 100, HIGH, 5);
    digitalWrite(LED_PIN, LOW);
  }     
}
*/
unsigned long tmrRandom() {
  randomSeed(analogRead(RANDOM_PIN));
  return random(24) * 5000L + 10000L;   // min 10sec, max 2min + 10sec   
}
void resetMe() {
  //wdt_enable(WDTO_15MS);
  //while(true); 
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
