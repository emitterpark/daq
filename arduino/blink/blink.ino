// Bootloader: Board SparkFun Pro Micro 3.3V 8MHZ
// sparkfun boards.txt 
// promicro.menu.cpu.8MHzatmega32U4.bootloader.extended_fuses=0xFF
// promicro.build.extra_flags={build.usb_flags} "-DUSB_VERSION=0x210"
// or C:Users/../AppData/Local/Arduino../packages/arduino/hardware/avr/../cores/arduino/USBCore.h 
// line 130: #define USB_VERSION 0x210

//#include <avr/wdt.h>
//#include <Timer.h>                // https://github.com/JChristensen/Timer
//#include <EEPROM.h>
//#include <CayenneLPP.h>           // https://github.com/ElectronicCats/CayenneLPP
//#include <Wire.h>
//#include <INA226.h>               // https://github.com/jarzebski/Arduino-INA226
//#include <WebUSB.h>               // https://github.com/webusb/arduino

const uint8_t AN_ALR_PIN[2]       = {12, 6};          // PD6/ADC9, PD7/ADC10
const uint8_t DIG_PIN[2]          = {8, 9};           // PB4/ADC11/PCINT4, PB5/ADC12/PCINT5
const uint8_t LORA_RES_PIN        = 10;               // PB6/ADC13/PCINT6
const uint8_t RANDOM_PIN          = A0;               // PF7/ADC7 
const uint8_t LED_PIN             = A5;               // PF0/ADC0

////WebUSB WebUSBSerial(1 /* https:// */, "emitterpark.github.io/daq");
//WebUSB WebUSBSerial(1 /* https:// */, "127.0.0.1:5500");
//#define usbSerial WebUSBSerial

void setup() {
  //wdt_enable(WDTO_8S);  
  setPin();   
  //setUsbSerial(); 
}
void loop() {  
  //readUsbSerial(); 
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500); 
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
/*
void readUsbSerial() {
  while (usbSerial && usbSerial.available()) {
    //wdt_reset();
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
        resetMe();
      } else if (strUsbSerial.startsWith(F("xdevice"))) {
        usbSerial.println(F("xdeviceeLoraWAN Wireless DAQ"));
        usbSerial.flush();
      } else if (strUsbSerial.startsWith(F("xversion"))) {
        usbSerial.println(F("xversionFirmware 1.0.1"));
        usbSerial.flush();
      } else if (strUsbSerial.startsWith(F("xfetch"))) {
        for (uint8_t ch = 0; ch < numAn; ch++) {
          fetchAnalog(ch);       
        }
        for (uint8_t ch = 0; ch < numDg; ch++) {
          fetchDigital(ch);       
        }
      } else if (strUsbSerial.startsWith(F("xchannels"))) {
        getChannels();      
      } else if (strUsbSerial.startsWith(F("xlorawan"))) {
        getLorawan();      
      }
      strUsbSerial = "";
    }
  }   
}
void setUsbSerial() {
  if (USBSTA >> VBUS & 1) {          
    while (!usbSerial) {
      //wdt_reset();
    }
  }  
  usbSerial.begin(9600);  
  usbSerial.flush();    
}
*/
