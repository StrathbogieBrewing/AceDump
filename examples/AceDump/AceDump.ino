#include "AceBMS.h"
#include "AceBus.h"
#include "AceDump.h"

#define LED_RED (3)
#define PWM_CHARGE (6)

#define kRxInterruptPin (2)
AceBus aceBus(Serial, kRxInterruptPin);

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(PWM_CHARGE, OUTPUT);
  analogWrite(PWM_CHARGE, 128);
  aceBus.begin();
}

void loop() {
  digitalWrite(LED_RED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_RED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}
