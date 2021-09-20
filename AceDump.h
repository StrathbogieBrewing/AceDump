#ifndef ACEDUMP_H
#define ACEDUMP_H

#include "sig.h"

#define ACEDUMP_STATUS (SIG_PRIORITY_MEDIUM | 0x50)
#define ACEDUMP_CONFIG (SIG_PRIORITY_MEDIUM | 0x60)

#define ACEDUMP_VBAT                                                           \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF0 | SIG_MILL | SIG_UINT)
#define ACEDUMP_DUTY                                                           \
  (ACEDUMP_STATUS | SIG_BYTE | SIG_OFF2 | SIG_CENT | SIG_UINT)
#define ACEDUMP_ENERGY                                                         \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF4 | SIG_MILL | SIG_UINT)
#define ACEDUMP_PERIOD                                                         \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF6 | SIG_MILL | SIG_UINT)

#define ACEDUMP_VSET                                                           \
  (ACEDUMP_CONFIG | SIG_WORD | SIG_OFF0 | SIG_MILL | SIG_UINT)

#define ACEDUMP_NAMES                                                          \
  {"Vdm", ACEDUMP_VBAT}, {"Ddm", ACEDUMP_DUTY}, {"Edm", ACEDUMP_ENERGY}, {     \
    "Fdm", ACEDUMP_PERIOD                                                      \
  }

#endif // ACEDUMP_H
