#pragma once
// POLYMORPHIC CONSTANTS - Generated per-build, DO NOT EDIT
#define POLY_XOR_KEY    0xFC
#define POLY_CFF_C1     22518
#define POLY_CFF_C2     12162
#define POLY_CFF_SHIFT  8
#define POLY_TIMING     17363202
#define POLY_CRC        0

// Junk code macro - random dead code injection
#define JUNK_CODE() do { \
    volatile __int64 _j = __rdtsc(); \
    _j = (_j * 22518) ^ (_j >> 5); \
    _j ^= (_j << 5); \
    if (_j == 0x59943CB) { _j ^= 0x7DEE03A2; } \
} while(0)

// Random opaque predicate constant
#define POLY_OPAQUE_PRED  (__rdtsc() % 29 != 0)
