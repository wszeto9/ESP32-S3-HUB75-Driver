#include "Arduino.h"
#ifndef HWTimer_h
#define HWTimer_h

class ScreenRefresh
{
  public:
    ScreenRefresh(int timer);
    void begin();
  private:
    int _pin;
};

#endif