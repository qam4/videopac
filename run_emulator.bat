@echo off
REM Windows batch script to run the Videopac emulator
REM Usage: run_emulator.bat [additional arguments]

set BIOS_PATH=roms\Philips C52 BIOS (19xx)(Philips)(FR).bin
set ROM_PATH=roms\Satellite Attack (1981)(Philips)(EU).bin

REM .\build\ci-win64\Release\videopac.exe --bios "%BIOS_PATH%" "%ROM_PATH%" %*
.\build\dev-mingw\videopac.exe --bios "%BIOS_PATH%" "%ROM_PATH%" %*
