#include <usbpd_def.h>
#include <usbpd_sink.h>

#define PD_EN PA2
#define PD_SENSE PA3
#define BARREL_EN PA0
#define BARREL_SENSE PA1
#define BUCK_EN PB3
#define LED_BUILTIN PB12

uint8_t myIndex = 0;
uint8_t setVoltage = REQUEST_20v;

void setup() {
  // put your setup code here, to run once:
  pinMode(BUCK_EN, OUTPUT);
  digitalWrite(BUCK_EN, LOW);

  pinMode(PD_EN, OUTPUT);
  digitalWrite(PD_EN, LOW);

  pinMode(BARREL_EN, OUTPUT);
  digitalWrite(BARREL_EN, LOW);

  pinMode(PD_SENSE, INPUT);

  usbpd_sink_init();

  for(int i = 0; i < 10; i++){
    if(usbpd_sink_get_ready()){
      usbpd_sink_set_request_fixed_voltage(setVoltage);
    }
    delay(10);
  }

  delay(20);
  digitalWrite(PD_EN, HIGH);
  delay(500);
  digitalWrite(BUCK_EN, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

}

void loop() {
  if(usbpd_sink_get_ready()){
      usbpd_sink_set_request_fixed_voltage(setVoltage);
  }
  uint16_t PD_Voltage = analogRead(PA3) >> 10;
  /*
  for(int counter = 0; counter < PD_Voltage; counter ++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  */
  delay(1000);
}
