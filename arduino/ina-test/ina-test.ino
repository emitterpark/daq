#include <Timer.h>
#include <Wire.h>
#include <INA226.h>
#include <WebUSB.h>

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

const uint8_t numAn               = 2;

float       an[numAn];

float res = 3.3, amp = 0.020;
uint8_t avg = 0b100, ctm = 0b111, pol = 5;

String strUsbSerial;
int pollTmr;

Timer t;
INA226 analog;
WebUSB WebUSBSerial(1 /* https:// */, "leanofis-iot.github.io/daq");
#define usbSerial WebUSBSerial

void setup() {
  setUsb();
  setPin();  
  setAnalog();    
  pollTmr = t.every(pol * 1000L, readAnalog);    
}
void loop() {  
  readUsbSerial();  
  t.update();
}
void readAnalog() {
  // wire.end();    
  for (uint8_t ch = 0; ch < numAn; ch++) {       
    while (AN_ALR_PIN[ch]) {       
    }
    analog.begin(0x40 + ch);
    an[ch] = analog.readShuntCurrent();
    if (analog.isAlert());
    usbSerial.println(an[ch], 5);                           
  }    
}
void readUsbSerial() {
  while (usbSerial && usbSerial.available()) {
    wdt_reset();
    const char chr = (char)usbSerial.read();
    strUsbSerial += chr;
    if (chr == '\n') {
      strUsbSerial.trim();
      const uint8_t valInt = strUsbSerial.substring(3).toInt();
      const float valFloat = strUsbSerial.substring(3).toFloat();       
      if (strUsbSerial.startsWith(F("avg"))) {
        avg = valInt;
      } else if (strUsbSerial.startsWith(F("ctm"))) {
        ctm = valInt;
      } else if (strUsbSerial.startsWith(F("res"))) {
        res = valFloat;
      } else if (strUsbSerial.startsWith(F("amp"))) {
        amp = valFloat; 
      } else if (strUsbSerial.startsWith(F("pol"))) {
        pol = valInt;         
      }
      strUsbSerial = "";
      setAnalog();
      //t.stop(pollTmr);    
      pollTmr = t.every(pol * 1000L, readAnalog);     
    }
  }   
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
    //analog.configure(INA226_AVERAGES_128, INA226_BUS_CONV_TIME_140US, INA226_SHUNT_CONV_TIME_8244US, INA226_MODE_SHUNT_CONT);
    analog.configure(avg, INA226_BUS_CONV_TIME_140US, ctm, INA226_MODE_SHUNT_CONT);
    analog.calibrate(res, amp);
    analog.enableConversionReadyAlert();               
  } 
}
void setUsb() {
  if (USBSTA >> VBUS & 1) {    
    usbSerial.begin(115200);    
    while (!usbSerial) {      
    }
    usbSerial.flush();
  } 
}
