param([string]$path = "build\emulator.exe")
if (-not (Test-Path $path)) { exit }

$bytes = [System.IO.File]::ReadAllBytes((Resolve-Path $path))
$e_lfanew = [System.BitConverter]::ToInt32($bytes, 0x3C)
$numSections = [System.BitConverter]::ToUInt16($bytes, $e_lfanew + 6)
$optHdrSize = [System.BitConverter]::ToUInt16($bytes, $e_lfanew + 20)
$sectionOffset = $e_lfanew + 24 + $optHdrSize
$changes = 0

# Rename sections: .text$mn -> .vmp0, .rdata -> .vmp1, .data -> .vmp2
for ($i = 0; $i -lt $numSections; $i++) {
    $off = $sectionOffset + $i * 40
    $name = [System.Text.Encoding]::ASCII.GetString($bytes, $off, 8).Replace([char]0, ' ').Trim()
    $newName = $null
    if ($name -eq '.text' -or $name -eq '.text$mn') { $newName = '.vmp0' }
    elseif ($name -eq '.rdata') { $newName = '.vmp1' }
    elseif ($name -eq '.data') { $newName = '.vmp2' }
    if ($newName) {
        $nb = [System.Text.Encoding]::ASCII.GetBytes($newName.PadRight(8, [char]0))
        [System.Array]::Copy($nb, 0, $bytes, $off, 8)
        $changes++
    }
}

# Section rename + UPX magic wipe only (string wipe REMOVED - CRT needs GetProcAddress/VirtualProtect)
if ($changes -gt 0) {
    [System.IO.File]::WriteAllBytes((Resolve-Path $path), $bytes)
}
Write-Host "Protection: $changes modifications"
