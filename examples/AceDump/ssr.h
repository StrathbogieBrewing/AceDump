#ifndef SSR_H_
#define SSR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ssr_kPatternLength (30)

#define ssr_kMaxDutyCycle (15)
#define ssr_kMinDutyCycle (0)

// needs to be called every 10 ms, returns SSR state history
uint8_t ssr_update(uint8_t duty);

// return SSR state history, b0 is current state
uint8_t ssr_getState(void);

// get sequence index counter
uint8_t ssr_busy(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SSR_H_ */
