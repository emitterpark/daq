#include <WebUSB.h>

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

String strUsbSerial, strLoraSerial;

WebUSB WebUSBSerial(1 /* https:// */, "leanofis-iot.github.io/daq");
#define usbSerial WebUSBSerial
//#define usbSerial Serial
#define loraSerial Serial1

void setup() {
  setPin();    
  setUsbSerial();
  setLoraSerial();  
}
void loop() {  
  readUsbSerial();
  readLoraSerial();  
}
void readUsbSerial() {
  while (usbSerial && usbSerial.available()) {    
    const char chr = (char)usbSerial.read();
    strUsbSerial += chr;
    if (chr == '\n') {
      strUsbSerial.trim();
      loraSerial.println(strUsbSerial);      
      strUsbSerial = "";           
    }
  }   
}
void readLoraSerial() {
  while (loraSerial.available()) {    
    const char chr = (char)loraSerial.read();
    strLoraSerial += chr;
    if (chr == '\n') {
      strLoraSerial.trim();
      usbSerial.println(strLoraSerial);
      usbSerial.flush();
      strLoraSerial = "";           
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
  digitalWrite(LORA_RES_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);  
}
void setUsbSerial() {     
  while (!usbSerial) {      
  }
  usbSerial.begin(9600); 
  usbSerial.flush();   
}
void setLoraSerial() {     
  loraSerial.begin(115200);  
}
