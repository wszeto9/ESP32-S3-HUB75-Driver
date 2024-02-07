#include "Arduino.h"
#include "HWTimer.h"

hw_timer_t * timer = NULL;


ScreenRefresh::ScreenRefresh(int pin){
  _pin = pin;
}

void IRAM_ATTR timerISR() {
  Serial.println("Interrupt occurred!"); // Print to serial
}

void ScreenRefresh::begin(){
  timer = timerBegin(_pin, 80, true); // Timer 0, divider 80
  timerAttachInterrupt(timer, &timerISR, true); // Attach ISR
  timerAlarmWrite(timer, 1000000, true); // 1s period
  timerAlarmEnable(timer); // Enable the timer
}

