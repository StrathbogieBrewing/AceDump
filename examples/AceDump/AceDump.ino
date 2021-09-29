#include <avr/wdt.h>

#include "PinChangeInterrupt.h"

#include "AceBMS.h"
#include "AceBus.h"
#include "AceDump.h"

#define VMAX (26900)
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

#define ZCD_TIMEOUT (250)

#define ENERGY_SCALE (182L)

#define kRxInterruptPin (2)
void aceCallback(tinframe_t *frame);
AceBus aceBus(Serial, kRxInterruptPin, aceCallback);

static unsigned long lastBMSUpdate = 0;
static uint16_t batmv = 0;
static uint16_t setmv = VSET;
static uint16_t batcv = 0;

static uint16_t ssrOn = 0;
static uint16_t ssrOff = 0;
static uint32_t energyCounter = 0;
static uint16_t energy = 0;
static uint16_t duty = 0;

static uint16_t linePeriod = 0;

static uint16_t redTimer = 0;
static uint16_t greenTimer = 0;
static uint16_t amberTimer = 0;

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
  batcv = ((uint32_t)(batmv + 5) * 6554L) >> 16;  // divide by 10
}

void update_1ms(void) {
  static unsigned long zcdLastMicros = 0;
  static uint8_t milliSeconds = 0;
  static bool positiveCycle = false;
  static uint16_t seconds = 0;

  update_adc(); // update batterry voltage
  aceBus.update();
  update_leds(); // update leds
  aceBus.update();

  if (milliSeconds < ZCD_TIMEOUT) {
    milliSeconds++;
  } else {
    redTimer = 50; // show no AC present
    milliSeconds = 50;
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

  aceBus.update();

  // if ((ssrOn & 0x0F) == 0x01)
  //   greenTimer = 20;

  if (seconds) {
    seconds--;
  } else {
    seconds = 999; // reset timer
    redTimer = 50; // show device is alive
    if ((ssrOn > 0) && (ssrOn < 320)) {
      duty = (100 * ssrOn) / (ssrOn + ssrOff);
      greenTimer = 50 + duty * 4;
    } else {
      duty = 0;
    }
    ssrOn = 0;
    ssrOff = 0;
  }

  energy = ((energyCounter * ENERGY_SCALE) >> 16L);
  if(energy > 8000L)  // limit to 8 kwh / day
        setmv = VMAX;

  aceBus.update();

  noInterrupts();
  unsigned long zcd = zcdMicros;
  interrupts();

  if (zcdLastMicros != zcd) { // check for zero crossing
    unsigned long delta = zcd - zcdLastMicros;
    if ((delta > 16000L) && (delta < 24000L))
      linePeriod = ((delta + 5) * 6554L) >> 16;  // divide by 10
    zcdLastMicros = zcd;
    if (milliSeconds > 3) // noise blanking for 3 ms
      milliSeconds = 0;   // then allow synchronisation with AC line
  } else {
    unsigned long delta = micros() - zcdLastMicros;
    if (delta > 100000L) {
      linePeriod = 0;
      energyCounter = 0;
      setmv = VSET;
    }
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
  if (sig_decode(msg, ACEBMS_RQST, &value) != FMT_NULL) {
    lastBMSUpdate = micros();
    if ((value & 0x0003) == 0)
      amberTimer = 50; // show rx data
    uint16_t frameSequence = value;
    if ((frameSequence & 0x03) == 0x02) {
      tinframe_t txFrame;
      msg_t *txMsg = (msg_t *)txFrame.data;
      sig_encode(txMsg, ACEDUMP_VBAT, batcv);
      sig_encode(txMsg, ACEDUMP_DUTY, duty);
      sig_encode(txMsg, ACEDUMP_ENERGY, energy);
      sig_encode(txMsg, ACEDUMP_PERIOD, linePeriod);
      aceBus.write(&txFrame);
    }
  }
  if (sig_decode(msg, ACEDUMP_VSET, &value) != FMT_NULL) {
    uint16_t mv = value * 10;
    if ((mv <= VMAX) && (mv >= VMIN)) {
      setmv = mv;
      aceBus.write(frame);  // acknowledgement
    }
  }
}
