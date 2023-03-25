#include "ultradist.h"

UltraDist::UltraDist(int _triggerPin, int _echoPin) {
  echoPin = _echoPin;
  triggerPin = _triggerPin;
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT_PULLDOWN);
}

void UltraDist::trigger() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
}

void UltraDist::attachHandler(voidFuncPtr handler) {
  attachInterrupt(digitalPinToInterrupt(echoPin), handler, CHANGE);
}

void UltraDistHandleInterrupt(UltraDist * ud) {
  u_long now = micros();
  byte currentState = digitalRead(ud->echoPin);
  if(currentState == HIGH) { // lastState == LOW && 
    ud->start = now;
  } else if(currentState == LOW) { // lastState == HIGH &&
    ud->dist = (now - ud->start) / ud->divisor;
  }
}