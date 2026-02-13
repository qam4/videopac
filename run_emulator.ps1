# PowerShell script to run the Videopac emulator
# Usage: .\run_emulator.ps1 [additional arguments]

$BiosPath = "roms\Philips C52 BIOS (19xx)(Philips)(FR).bin"
$RomPath = "roms\Satellite Attack (1981)(Philips)(EU).bin"

& ".\build\ci-win64\Release\videopac.exe" --bios $BiosPath $RomPath $args
