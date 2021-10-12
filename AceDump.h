#ifndef ACEDUMP_H
#define ACEDUMP_H

#include "sig.h"

#define ACEDUMP_STATUS (0x05 | SIG_SIZE7)
#define ACEDUMP_CONFIG (0x15 | SIG_SIZE2)

#define ACEDUMP_VBAT                                                           \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF0 | SIG_CENT | SIG_UINT)
#define ACEDUMP_ENERGY                                                         \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF2 | SIG_CENT | SIG_UINT)
#define ACEDUMP_PERIOD                                                         \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF4 | SIG_DECI | SIG_UINT)
#define ACEDUMP_DUTY                                                           \
  (ACEDUMP_STATUS | SIG_BYTE | SIG_OFF6 | SIG_UNIT | SIG_UINT)

#define ACEDUMP_VSET                                                           \
  (ACEDUMP_CONFIG | SIG_WORD | SIG_OFF0 | SIG_CENT | SIG_UINT | SIG_RW)

#define ACEDUMP_NAMES                                                          \
  {"dVb", ACEDUMP_VBAT}, {"dDty", ACEDUMP_DUTY},                    \
      {"dkwh", ACEDUMP_ENERGY}, {"dac", ACEDUMP_PERIOD}, {      \
    "dVs", ACEDUMP_VSET                                                  \
  }

#endif // ACEDUMP_H
