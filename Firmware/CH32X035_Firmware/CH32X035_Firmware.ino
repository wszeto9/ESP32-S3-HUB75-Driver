#define REV_B02 //revision defines what pins are being used

#include "CH32X035_Pinout.h"
#include <usbpd_def.h>
#include <usbpd_sink.h>

uint8_t setVoltage = REQUEST_20v;

void PinInit(){
  pinMode(BUCK_EN, OUTPUT);
  digitalWrite(BUCK_EN, LOW);
  pinMode(PD_EN, OUTPUT);
  digitalWrite(PD_EN, LOW);
  pinMode(BARREL_EN, OUTPUT);
  digitalWrite(BARREL_EN, LOW);
  pinMode(BARREL_SENSE, INPUT);
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
  if((analogRead(BARREL_SENSE)>>7) > 5){
    digitalWrite(BARREL_EN, HIGH); //uses barrel power if barrel power is over 5V
  }
  else{
    digitalWrite(PD_EN, HIGH);
  }
  delay(2); //determined experimentally: HSS transient over in 1ms
  digitalWrite(BUCK_EN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if(usbpd_sink_get_ready()){
      usbpd_sink_set_request_fixed_voltage(setVoltage);
  }
  uint8_t PD_Voltage = 1 + analogRead(PD_SENSE) >> 7; //Rev B01: 3.3K low, 18K high. CH32 has no divide instuction set, so we need to right shift to multiply / divide  
  if(PD_Voltage == 8){
    PD_Voltage = 0x05; // when VBUS is 5V, VCC of CH32X035 is closer to 3.5V. This means that the analogRead is based on full scale of 3.5V, not 5V. This compensates for this affect.   
  }
  for(uint8_t counter = 0; counter < PD_Voltage; counter ++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  delay(1000);
}
