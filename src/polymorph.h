#pragma once
// POLYMORPHIC CONSTANTS - Generated per-build, DO NOT EDIT
#define POLY_XOR_KEY    0x83
#define POLY_CFF_C1     58043
#define POLY_CFF_C2     54111
#define POLY_CFF_SHIFT  16
#define POLY_TIMING     18234658
#define POLY_CRC        0

// Junk code macro - random dead code injection
#define JUNK_CODE() do { \
    volatile __int64 _j = __rdtsc(); \
    _j = (_j * 58043) ^ (_j >> 5); \
    _j ^= (_j << 8); \
    if (_j == 0x5A97DB70) { _j ^= 0x4B56418; } \
} while(0)

// Random opaque predicate constant
#define POLY_OPAQUE_PRED  (__rdtsc() % 12 != 0)
