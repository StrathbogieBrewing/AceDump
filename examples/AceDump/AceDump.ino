#include <avr/wdt.h>

#include "PinChangeInterrupt.h"

#include "AceBMS.h"
#include "AceBus.h"
#include "AceDump.h"
#include "ssr.h"

#define VMAX (27000)
#define VMIN (26500)

#define LED_RED (3)
#define LED_GREEN (4)
#define LED_AMBER (5)

#define ATEN_0 (16)
#define ATEN_1 (17)

#define PWM_CHARGE (6)

#define SSR_DRIVE (9)
#define ZCD_DETECT (8)

#define VBAT_SENSE (A1)

#define kRxInterruptPin (2)
void aceCallback(tinframe_t *frame);
AceBus aceBus(Serial, kRxInterruptPin, aceCallback);

static unsigned long lastBMSUpdate = 0;
static int dutyCycle = 0;
static int batmv = 0;
static int ssrState = 0;

static volatile unsigned char zcdCounter = 0;
void zcdTriggered(void) { zcdCounter++; }

void zcdUpdate(void) {
  static unsigned long lastMillis = 0;
  static unsigned char lastCounter = 0;

  unsigned long newMillis = millis();
  unsigned long periodMillis = newMillis - lastMillis;

  if(periodMillis > 25){
    digitalWrite(LED_RED, HIGH);  // show no AC present
  }

  if (lastCounter == zcdCounter)
    return; // no change
  lastCounter = zcdCounter;

  lastMillis = newMillis;

  if (periodMillis < 5)
    return; // blank spurious zero crossings less than 5 ms

  if ((periodMillis > 15) && (periodMillis < 25)) {
    ssrState = ssr_update(dutyCycle) & 0x01;  // update ssr if period is reasonable
  }
}

void setup() {
  wdt_disable();
  delay(2000);
  wdt_enable(WDTO_250MS);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_AMBER, OUTPUT);

  pinMode(PWM_CHARGE, OUTPUT);
  analogWrite(PWM_CHARGE, 128);

  digitalWrite(SSR_DRIVE, LOW);
  pinMode(SSR_DRIVE, OUTPUT);

  digitalWrite(ATEN_0, LOW); // for 24 V voltage sensing
  pinMode(ATEN_0, OUTPUT);
  digitalWrite(ATEN_1, HIGH);
  pinMode(ATEN_1, OUTPUT);

  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(ZCD_DETECT),
                           zcdTriggered, RISING);
  aceBus.begin();
}

void loop() {
  wdt_reset();
  zcdUpdate();
  aceBus.update();

  uint16_t adc = analogRead(VBAT_SENSE);  // use local vbat for override
  if (adc > 800)
    adc = 800;       // limit adc to 32 V
  batmv = adc * 40;  // convert to mv

  if (batmv < VMIN){
    ssrState = LOW;  // under voltage so force off
    dutyCycle = ssr_kMinDutyCycle;
  } else if (batmv > VMAX){
    ssrState = HIGH;  // over voltage so force on
    dutyCycle = ssr_kMaxDutyCycle;
  } else {
    dutyCycle = (batmv - VMIN) / 10;
  }

  digitalWrite(SSR_DRIVE, ssrState);
  digitalWrite(LED_GREEN, ssrState);

  digitalWrite(LED_RED, millis() & 0x80); // show device is alive
}

void aceCallback(tinframe_t *frame) {
  msg_t *msg = (msg_t *)(frame->data);
  int16_t value;
  if (sig_decode(msg, ACEBMS_VBAT, &value) != FMT_NULL) {
    uint32_t senseVoltage = (value + 5) / 10 - 10;
    digitalWrite(LED_AMBER, !digitalRead(LED_AMBER));  // show rx data
    lastBMSUpdate = millis();
  }
  if (sig_decode(msg, ACEBMS_RQST, &value) != FMT_NULL) {
    uint8_t frameSequence = value;
    if ((frameSequence & 0x03) == 0x02) {
      tinframe_t txFrame;
      msg_t *txMsg = (msg_t *)txFrame.data;
      sig_encode(txMsg, ACEDUMP_VBAT, batmv);
      sig_encode(txMsg, ACEDUMP_DUTY, dutyCycle);
      if (aceBus.write(&txFrame) != AceBus_kOK) {
      }
    }
  }
}
