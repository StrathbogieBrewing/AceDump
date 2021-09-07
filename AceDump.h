#ifndef ACEDUMP_H
#define ACEDUMP_H

#include "sig.h"

#define ACEDUMP_STATUS (SIG_PRIORITY_MEDIUM | 0x50)

#define ACEDUMP_VPV2                                                      \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF0 | SIG_CENT | SIG_UINT)
#define ACEDUMP_ICH2                                                      \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF2 | SIG_DECI | SIG_UINT)
#define ACEDUMP_VPV3                                                      \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF4 | SIG_CENT | SIG_UINT)
#define ACEDUMP_ICH3                                                      \
  (ACEDUMP_STATUS | SIG_WORD | SIG_OFF6 | SIG_DECI | SIG_UINT)

#define ACEDUMP_NAMES                                                     \
  {"Vpv2", ACEDUMP_VPV2}, {"Ich2", ACEDUMP_ICH2},                    \
      {"Vpv3", ACEDUMP_VPV3}, {"Ich3", ACEDUMP_ICH3},

#endif // ACEDUMP_H
