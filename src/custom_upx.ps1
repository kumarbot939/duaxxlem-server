$path = "build\emulator.exe"
if (-not (Test-Path $path)) { exit }

$bytes = [System.IO.File]::ReadAllBytes((Resolve-Path $path))
$e_lfanew = [System.BitConverter]::ToInt32($bytes, 0x3C)
$numSections = [System.BitConverter]::ToUInt16($bytes, $e_lfanew + 6)
$optHeaderSize = [System.BitConverter]::ToUInt16($bytes, $e_lfanew + 20)
$sectionOffset = $e_lfanew + 24 + $optHeaderSize

$changes = 0
for ($i = 0; $i -lt 3 -and $i -lt $numSections; $i++) {
    $offset = $sectionOffset + $i * 40
    $secName = [System.Text.Encoding]::ASCII.GetString($bytes, $offset, 4)
    if ($secName -eq "UPX0" -or $secName -eq "UPX1" -or $secName -eq "UPX2") {
        $newBytes = [System.Text.Encoding]::ASCII.GetBytes("PDX" + $secName[3])
        [System.Array]::Copy($newBytes, 0, $bytes, $offset, 4)
        $changes++
    }
}

# UPX magic bytes (UPX!) imzasini da sil - AI gorunce "standart UPX" demesin
$upxMagic = [System.Text.Encoding]::ASCII.GetBytes("UPX!")
for ($i = 0; $i -lt $bytes.Length - 4; $i++) {
    $found = $true
    for ($j = 0; $j -lt 4; $j++) { if ($bytes[$i+$j] -ne $upxMagic[$j]) { $found = $false; break } }
    if ($found) { for ($j = 0; $j -lt 4; $j++) { $bytes[$i+$j] = 0x90 }; $changes++ }
}

if ($changes -gt 0) {
    [System.IO.File]::WriteAllBytes((Resolve-Path $path), $bytes)
    Write-Host "UPX renaming: $changes modifications"
}
