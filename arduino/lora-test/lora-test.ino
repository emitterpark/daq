//#include <WebUSB.h>

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

String strUsbSerial, strSerial1;

//WebUSB WebUSBSerial(1 /* https:// */, "leanofis-iot.github.io/daq");
//#define usbSerial WebUSBSerial
#define usbSerial Serial
//#define loraSerial Serial1

void setup() {
  setPin();    
  setUsbSerial();
  setSerial1();  
}
void loop() {  
  readUsbSerial();
  readSerial1();  
}
void readUsbSerial() {
  while (usbSerial && usbSerial.available()) {    
    const char chr = (char)usbSerial.read();
    strUsbSerial += chr;
    if (chr == '\n') {
      strUsbSerial.trim();
      Serial1.println(strUsbSerial);
      usbSerial.println(strUsbSerial);
      strUsbSerial = "";           
    }
  }   
}
void readSerial1() {
  while (Serial1.available()) {    
    const char chr = (char)Serial1.read();
    strSerial1 += chr;
    if (chr == '\n') {
      strSerial1.trim();
      usbSerial.println(strSerial1);
      strSerial1 = "";           
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
  if (USBSTA >> VBUS & 1) {    
    usbSerial.begin(115200);    
    while (!usbSerial) {      
    }
    usbSerial.flush();
    usbSerial.println("hi there");
    usbSerial.println("this is me daq");
  } 
}
void setSerial1() {     
  Serial1.begin(115200);    
  while (!Serial1) {      
  }   
}
