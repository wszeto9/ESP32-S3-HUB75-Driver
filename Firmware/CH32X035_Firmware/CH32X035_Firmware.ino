#include "CH32X035_Pinout.h"
#include <usbpd_def.h>
#include <usbpd_sink.h>

#define REV_B02 1

uint8_t setVoltage = REQUEST_20v;

void PinInit(){
  pinMode(BUCK_EN, OUTPUT);
  digitalWrite(BUCK_EN, LOW);

  pinMode(PD_EN, OUTPUT);
  digitalWrite(PD_EN, LOW);

  pinMode(BARREL_EN, OUTPUT);
  digitalWrite(BARREL_EN, LOW);

  pinMode(PD_SENSE, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {

  PinInit();
  usbpd_sink_init();
  delay(100);

  for(int i = 0; i < 10; i++){
    if(usbpd_sink_get_ready()){
      usbpd_sink_set_request_fixed_voltage(setVoltage);
    }
    delay(100); //PD chargers range from 100ms to 200ms to slew to 20V from 5V in my testing
    if((analogRead(PD_SENSE) >> 7) > 9){
      break;
    }
  }
  delay(30); //just to be safe... 
  digitalWrite(PD_EN, HIGH);
  delay(2); //determined experimentally: HSS transient over in 1ms
  digitalWrite(BUCK_EN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if(usbpd_sink_get_ready()){
      usbpd_sink_set_request_fixed_voltage(setVoltage);
  }

  uint8_t PD_Voltage = analogRead(PD_SENSE) >> 7; //Rev B01: 3.3K low, 18K high. CH32 has no divide instuction set, so we need to right shift to multiply / divide  
  
  for(uint8_t counter = 0; counter < PD_Voltage; counter ++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  
  delay(1000);
}
