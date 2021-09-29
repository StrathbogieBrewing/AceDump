#ifndef ACEDUMP_H
#define ACEDUMP_H

#include "sig.h"

#define ACEDUMP_STATUS (SIG_PRIORITY_MEDIUM | 0x50)
#define ACEDUMP_CONFIG (SIG_PRIORITY_HIGHMED | 0x50)

#define ACEDUMP_VBAT                                                           \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF0 | SIG_CENT | SIG_UINT)
#define ACEDUMP_DUTY                                                           \
  (ACEDUMP_STATUS | SIG_BYTE | SIG_OFF2 | SIG_UNIT | SIG_UINT)
#define ACEDUMP_ENERGY                                                         \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF4 | SIG_UNIT | SIG_UINT)
#define ACEDUMP_PERIOD                                                         \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF6 | SIG_CENT | SIG_UINT)

#define ACEDUMP_VSET                                                           \
  (ACEDUMP_CONFIG | SIG_WORD | SIG_OFF0 | SIG_CENT | SIG_UINT | SIG_RW)

#define ACEDUMP_NAMES                                                          \
  {"oven/vbat", ACEDUMP_VBAT}, {"oven/duty", ACEDUMP_DUTY},                    \
      {"oven/energy", ACEDUMP_ENERGY}, {"oven/period", ACEDUMP_PERIOD}, {      \
    "oven/vset", ACEDUMP_VSET                                                  \
  }

#endif // ACEDUMP_H
