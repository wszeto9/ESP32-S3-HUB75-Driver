#define REV_B02 //revision defines what pins are being used

#include "CH32X035_Pinout.h"
#include <usbpd_def.h>
#include <usbpd_sink.h>

uint8_t setVoltage = REQUEST_20v;

enum PowerSourceTypes {USBC, BARREL};

PowerSourceTypes PowerSource;

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

void processPowerSwap(PowerSourceTypes targetPowerSource){
  if(targetPowerSource == USBC){
    digitalWrite(BARREL_EN, LOW);
    digitalWrite(PD_EN, HIGH);
    PowerSource = USBC;
    delay(200);
  }
  else{
    digitalWrite(PD_EN, LOW);
    digitalWrite(BARREL_EN, HIGH);
    PowerSource = BARREL;
    delay(200);
  }
}

void delayWithPowerChecking(int delay_ms){
  uint16_t PD_Voltage_measured;
  uint16_t Barrel_Voltage_measured;
  for(int counter = 0; counter < delay_ms; counter++){
    PD_Voltage_measured = 1 + analogRead(PD_SENSE) >> 7;
    if(PD_Voltage_measured == 8){
        PD_Voltage_measured = 0x05; // when VBUS is 5V, VCC of CH32X035 is closer to 3.5V. This means that the analogRead is based on full scale of 3.5V, not 5V. This compensates for this affect.   
    }

    Barrel_Voltage_measured = analogRead(BARREL_SENSE);
    if((PD_Voltage_measured < 3) && PowerSource == USBC){
      processPowerSwap(BARREL);
    }
    else if((Barrel_Voltage_measured < 300) && PowerSource == BARREL){
      processPowerSwap(USBC);
    }
    delay(1);
  }
}

void setup() {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  PWR_PVDLevelConfig(PWR_PVDLevel_4V0);
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
  if((analogRead(BARREL_SENSE)) > 300){
    digitalWrite(BARREL_EN, HIGH); //uses barrel power if barrel power is over 5V
    PowerSource = BARREL;
  }
  else{
    digitalWrite(PD_EN, HIGH);
    PowerSource = USBC;
  }
  delay(2); //determined experimentally: HSS transient over in 1ms
  digitalWrite(BUCK_EN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if(usbpd_sink_get_ready()){
      usbpd_sink_set_request_fixed_voltage(setVoltage);
  }
  if(PowerSource == USBC){
    uint8_t InputVoltage;
    if(PWR_GetFlagStatus(PWR_FLAG_PVDO)){
      InputVoltage = 3;
    }
    else{
      InputVoltage = 5;
    }
    float PD_Voltage = (float) analogRead(PD_SENSE) * SenseDividerRatio * InputVoltage / 4095; //Rev B01: 3.3K low, 18K high. CH32 has no divide instuction set, so we need to right shift to multiply / divide  
    PD_Voltage = PD_Voltage * 1.12 - 0.2156; //fudge factors
    if(PWR_GetFlagStatus(PWR_FLAG_PVDO)){
      PD_Voltage = 0x05; // when VBUS is 5V, VCC of CH32X035 is closer to 3.5V. This means that the analogRead is based on full scale of 3.5V, not 5V. This compensates for this affect.   
    }
    for(uint8_t counter = 0; counter < (uint8_t) round(PD_Voltage); counter ++){
      digitalWrite(LED_BUILTIN, HIGH);
      delayWithPowerChecking(200);
      digitalWrite(LED_BUILTIN, LOW);
      delayWithPowerChecking(200);
    }
    delayWithPowerChecking(1000);
  }
  else{
      digitalWrite(LED_BUILTIN, HIGH);
      delayWithPowerChecking(200);
      digitalWrite(LED_BUILTIN, LOW);
      delayWithPowerChecking(200);
  }

}
