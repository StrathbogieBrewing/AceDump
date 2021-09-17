#include <avr/wdt.h>

#include "PinChangeInterrupt.h"

#include "AceBMS.h"
#include "AceBus.h"
#include "AceDump.h"

#define VMAX (27000)
#define VMIN (26500)
#define VSET (26700)

#define LED_RED (3)
#define LED_GREEN (4)
#define LED_AMBER (5)

#define ATEN_0 (16)
#define ATEN_1 (17)

#define PWM_CHARGE (6)

#define SSR_DRIVE (9)
#define ZCD_DETECT (8)

#define VBAT_SENSE (A1)

#define ZCD_TIMEOUT (100)

#define ENERGY_SCALE (182L)

#define kRxInterruptPin (2)
void aceCallback(tinframe_t *frame);
AceBus aceBus(Serial, kRxInterruptPin, aceCallback);

static unsigned long lastBMSUpdate = 0;
static uint16_t batmv = 0;
static uint16_t setmv = VSET;

static uint16_t ssrOn = 0;
static uint16_t ssrOff = 0;
static uint32_t energyCounter = 0;

static uint16_t linePeriod = 0;

static int16_t redTimer = 0;
static int16_t greenTimer = 0;
static int16_t amberTimer = 0;

static volatile unsigned long zcdMicros = 0;
void zcdTriggered(void) { zcdMicros = micros(); }

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
                           zcdTriggered, FALLING);
  aceBus.begin();
}

void update_leds(void) {
  if (redTimer > 0) {
    redTimer -= 1;
    digitalWrite(LED_RED, HIGH);
  } else {
    digitalWrite(LED_RED, LOW);
  }

  if (greenTimer > 0) {
    greenTimer -= 1;
    digitalWrite(LED_GREEN, HIGH);
  } else {
    digitalWrite(LED_GREEN, LOW);
  }

  if (amberTimer > 0) {
    amberTimer -= 1;
    digitalWrite(LED_AMBER, HIGH);
  } else {
    digitalWrite(LED_AMBER, LOW);
  }
}

void update_adc(void) {
  static uint16_t adc_filter = 0;
  uint16_t adc = analogRead(VBAT_SENSE); // use local vbat for control
  if (adc > 800)
    adc = 800; // limit adc to 32 V
  adc_filter -= (adc_filter >> 6);
  adc_filter += adc;
  batmv = (adc_filter >> 3) * 5; // convert to mv
}

void update_1ms(void) {
  static unsigned long zcdLastMicros = 0;
  static uint8_t milliSeconds = 0;
  static bool positiveCycle = false;
  static uint16_t seconds = 0;

  update_adc(); // update batterry voltage
  update_leds();

  if (milliSeconds < ZCD_TIMEOUT) {
    milliSeconds++;
  } else {
    redTimer = 500; // show no AC present
  }

  if (milliSeconds == 5) {

    if (batmv < setmv) { // update ssr output
      digitalWrite(SSR_DRIVE, LOW);
      ssrOff++;
    } else {
      if (positiveCycle == true) {
        positiveCycle = false;
        digitalWrite(SSR_DRIVE, HIGH);
        ssrOn++;
        energyCounter++;
      }
    }
  }

  if (milliSeconds == 15) {
    if (batmv < setmv) { // update ssr output
      digitalWrite(SSR_DRIVE, LOW);
      ssrOff++;
    } else {
      if (positiveCycle == false) {
        positiveCycle = true;
        digitalWrite(SSR_DRIVE, HIGH);
        ssrOn++;
        energyCounter++;
      }
    }
  }

  if ((ssrOn & 0x0F) == 0x01)
    greenTimer = 20;

  if (seconds) {
    seconds--;
  } else {
    seconds = 999; // reset timer
    redTimer = 50; // show device is alive
  }

  if (zcdLastMicros != zcdMicros) { // check for zero crossing
    noInterrupts();
    unsigned long delta = zcdMicros - zcdLastMicros;
    interrupts();
    if ((delta > 18000L) && (delta < 22000L))
      linePeriod = delta;
    zcdLastMicros = zcdMicros;
    if (milliSeconds > 3) // noise blanking for 3 ms
      milliSeconds = 0;   // then allow synchronisation with AC line
  }
}

void loop() {
  wdt_reset();
  aceBus.update();

  static unsigned long time = 0; // assume max loop time is less than 1 ms
  unsigned long now = micros();
  if (now >= time + 1000) {
    update_1ms();
    if (now >= time + 1500)
      time = now;
    else
      time += 1000;
  }
}

void aceCallback(tinframe_t *frame) {
  msg_t *msg = (msg_t *)(frame->data);
  int16_t value;
  if (sig_decode(msg, ACEBMS_VBAT, &value) != FMT_NULL) {
    uint32_t senseVoltage = (value + 5) / 10 - 10;
    lastBMSUpdate = micros();
  }
  if (sig_decode(msg, ACEBMS_RQST, &value) != FMT_NULL) {
    if ((value & 0x0003) == 0)
      amberTimer = 50; // show rx data
    uint8_t frameSequence = value;
    if ((frameSequence & 0x03) == 0x02) {
      tinframe_t txFrame;
      msg_t *txMsg = (msg_t *)txFrame.data;
      sig_encode(txMsg, ACEDUMP_VBAT, batmv);
      uint16_t duty = 0;
      if ((ssrOn > 0) && (ssrOn < 320)) {
        duty = (100 * ssrOn) / (ssrOn + ssrOff);
      }
      ssrOn = 0;
      ssrOff = 0;
      sig_encode(txMsg, ACEDUMP_DUTY, duty);
      sig_encode(txMsg, ACEDUMP_ENERGY,
                 ((energyCounter * ENERGY_SCALE) >> 16L));
      sig_encode(txMsg, ACEDUMP_PERIOD, linePeriod);
      aceBus.write(&txFrame);
    }
  }
  if (sig_decode(msg, ACEDUMP_VSET, &value) != FMT_NULL) {
    if ((value <= VMAX) && (value >= VMIN)) {
      setmv = value;
    }
  }
}
