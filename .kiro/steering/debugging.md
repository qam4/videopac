---
inclusion: manual
---

# Videopac Debugging Tools

## Available Tools

### videopac --headless --trace

Boot trace that dumps per-instruction CPU/VDC state.

```
./build/dev-mingw/videopac.exe --bios roms/BIOS/bios_O2rom.bin --headless --frames 120 --press-key 1 3 --trace --vdc-trace --region usa "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

### run_emulator.py hl

Headless mode with screenshots and trace. Config is in the script: `--frames 120`, `--press-key 1 3`, `--region france`, `--trace`, `--vdc-trace`.

```
python run_emulator.py hl "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

### videopac --debug

Interactive debugger with breakpoints, step execution, memory dump, disassembly.

Conditional breakpoints:
- CPU state: `--break 0x09F --condition "cpu.A == 0xFF"`
- VDC state: `--break 0x127 --condition "vdc.display_enabled == true"`
- Memory state: `--break 0x500 --condition "memory.external_ram[0x20] > 0x80"`

### disasm_tool

MCS-48 ROM disassembler with automatic label generation.

```
./build/dev-mingw/disasm_tool.exe "roms/Satellite Attack (1981)(Philips)(EU).bin" --rom
./build/dev-mingw/disasm_tool.exe "roms/BIOS/bios_O2rom.bin" > debug/bios_disasm.txt
```

## Key Addresses (MCS-48 / Intel 8048)

- `0x003` — External interrupt vector (VBlank)
- `0x007` — Timer interrupt vector
- `0x0B0` — BIOS keyboard scan routine
- `0x13D` — BIOS get_keystroke (blocks until key pressed)
- `0x176` — BIOS wait_for_interrupt (waits for VBlank via F1 flag)
- `0x2C3` — BIOS select_game routine
- `0x400` — Cartridge entry point
- `0x408` — Game start vector (after select_game returns)

## Hardware Reference

- `doc/hardware/odyssey2_timing.txt` — NTSC timing (verified on real hardware)
- `doc/reference/o2doc.md` — VDC register map, sprite/character/grid rendering
- `doc/reference/mcs-48-user-manual.md` — CPU instruction set
- `doc/hardware/8245.md` — Intel 8245 VDC datasheet notes
