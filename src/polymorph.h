#pragma once
// POLYMORPHIC CONSTANTS - Generated per-build, DO NOT EDIT
#define POLY_XOR_KEY    0xD5
#define POLY_CFF_C1     62382
#define POLY_CFF_C2     31794
#define POLY_CFF_SHIFT  8
#define POLY_TIMING     16174398

// Junk code macro - random dead code injection
#define JUNK_CODE() do { \
    volatile __int64 _j = __rdtsc(); \
    _j = (_j * 62382) ^ (_j >> 7); \
    _j ^= (_j << 5); \
    if (_j == 0x5B884C98) { _j ^= 0x3FB76E0B; } \
} while(0)

// Random opaque predicate constant
#define POLY_OPAQUE_PRED  (__rdtsc() % 14 != 0)
