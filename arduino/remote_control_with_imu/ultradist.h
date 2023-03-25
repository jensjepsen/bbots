// LED.h
#ifndef ultradist_h
#define ultradist_h

#include <Arduino.h>

class UltraDist {
  private:
    
  public:
    UltraDist(int _triggerPin, int _echoPin);
    void trigger();
    volatile u_long start = 0;
    volatile u_long dist = 0;
    int triggerPin;
    int echoPin;
    volatile int divisor = 29 / 2;
    void attachHandler(voidFuncPtr handler);
};

void UltraDistHandleInterrupt(UltraDist * ud);


#endif