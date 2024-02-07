#include "HWTimer.h"
  
ScreenRefresh ScreenRefresh(0);

void setup() {
  Serial.begin(115200);
  delay(1000); // Give some time for serial to initialize
  ScreenRefresh.begin();
}

void loop() {
  // Main program loop
}
