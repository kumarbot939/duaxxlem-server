# Polymorphic pre-build generator - her build farkli binary icin
$rand = [System.Random]::new()

# Generate entropy_pad.h (32KB random data, farkli her build)
$pad = @()
$pad += "#pragma once"
$pad += "static const unsigned char g_entropyPad[" + (32768) + "] = {"
for ($i = 0; $i -lt 32768; $i += 16) {
    $line = "    "
    for ($j = 0; $j -lt 16 -and ($i + $j) -lt 32768; $j++) {
        $line += "0x" + $rand.Next(0, 256).ToString("X2") + ","
    }
    $pad += $line
}
$pad += "};"
[System.IO.File]::WriteAllLines("src\entropy_pad.h", $pad)

# Generate polymorph.h with random constants
$xorKey = $rand.Next(1, 255)  # Random XOR key for string ekstra layer
$cffConst1 = $rand.Next(0, 65536)
$cffConst2 = $rand.Next(0, 65536)
$cffShift = $rand.Next(2, 6) * 4  # 8, 12, 16, 20
$timingThreshold = $rand.Next(5000000, 30000000)
$junkN = $rand.Next(10, 50)

$poly = @()
$poly += "#pragma once"
$poly += "// POLYMORPHIC CONSTANTS - Generated per-build, DO NOT EDIT"
$poly += "#define POLY_XOR_KEY    0x" + $xorKey.ToString("X2")
$poly += "#define POLY_CFF_C1     " + $cffConst1
$poly += "#define POLY_CFF_C2     " + $cffConst2
$poly += "#define POLY_CFF_SHIFT  " + $cffShift
$poly += "#define POLY_TIMING     " + $timingThreshold
$poly += "#define POLY_CRC        0"
$poly += ""
$poly += "// Junk code macro - random dead code injection"
$poly += "#define JUNK_CODE() do { \"
$poly += "    volatile __int64 _j = __rdtsc(); \"
$poly += "    _j = (_j * " + $cffConst1 + ") ^ (_j >> " + ($rand.Next(3,9)) + "); \"
$poly += "    _j ^= (_j << " + ($rand.Next(3,9)) + "); \"
$poly += "    if (_j == 0x" + $rand.Next(0, [int]::MaxValue).ToString("X") + ") { _j ^= 0x" + $rand.Next(0, [int]::MaxValue).ToString("X") + "; } \"
$poly += "} while(0)"
$poly += ""
$poly += "// Random opaque predicate constant"
$poly += "#define POLY_OPAQUE_PRED  (__rdtsc() % " + $junkN + " != 0)"

[System.IO.File]::WriteAllLines("src\polymorph.h", $poly)

Write-Host "Prebuild: entropy_pad.h + polymorph.h generated (XOR key: 0x$($xorKey.ToString('X2')))"
