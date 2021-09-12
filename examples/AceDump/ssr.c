#ifdef __AVR__

#include "avr/io.h"
#include <avr/pgmspace.h>

#else

#define PROGMEM
#define pgm_read_word(x) (*(x))

#endif

#include "ssr.h"

const uint16_t ssrTable[ssr_kMaxDutyCycle + 1] PROGMEM = {
    0b000000000000000, // 0%
    0b000000010000000, // 7%
    0b000100000001000, // 13%
    0b001000010000100, // 20%
    0b010001000100010, // 27%
    0b010010010010010, // 33%
    0b010100101001010, // 40%
    0b010101010101010, // 47%
    0b101010101010101, // 53%
    0b101011010110101, // 60%
    0b101101101101101, // 67%
    0b101110111011101, // 73%
    0b110111101111011, // 80%
    0b111011111110111, // 87%
    0b111111101111111, // 93%
    0b111111111111111  // 100%
};

static uint8_t ssr_state = 0;
static uint8_t ssr_index = 0;

uint8_t ssr_update(uint8_t duty) {
  static int16_t mask = 0;

  if (ssr_index) {
    // update ssr output
    ssr_index--;
    if ((ssr_index & 0x01)) {
      mask >>= 1;
    }
  } else {
    // reload ssr pattern
    if (duty > ssr_kMaxDutyCycle) {
      duty = ssr_kMaxDutyCycle;
    }
    mask = pgm_read_word(&ssrTable[duty]);
    ssr_index = ssr_kPatternLength - 1;
  }

  ssr_state <<= 1;
  ssr_state |= (mask & 0x01);

  return ssr_state;
}

uint8_t ssr_busy(void) { return ssr_index; }

uint8_t ssr_getState(void) { return ssr_state; }
