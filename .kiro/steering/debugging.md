---
inclusion: manual
---

# Emulator Debugging Methodology

## Principles

1. **The ROM is ground truth.** It defines what the hardware does. Don't assume wiring from datasheets — verify against the actual ROM code.
2. **Don't guess at the code level.** If a fix feels like a workaround, the root cause is probably elsewhere.
3. **Enhance existing tools, don't create new ones.** Check what's available below before writing throwaway scripts.

## Available Tools

### videopac --headless --trace

Boot trace that dumps per-instruction CPU/VDC state. Use for verifying interrupt handling, boot sequence, keyboard input, beam position, etc.

```
./build/dev-mingw/videopac.exe --bios roms/BIOS/bios_O2rom.bin --headless --frames 120 --press-key 1 3 --trace --vdc-trace --region usa "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

Output includes PC, A, PSW, P1, P2, register bank, VDC beam position, control/status registers, display state, and memory access details per instruction.

### run_emulator.py hl

Headless mode with screenshots and trace. Generates PNGs every frame and writes trace logs.

```
python run_emulator.py hl "roms/Satellite Attack (1981)(Philips)(EU).bin"
```

Configuration is in the script: `--frames 120`, `--press-key 1 3`, `--region france`, `--trace`, `--vdc-trace`. Edit as needed.

### videopac --debug

Interactive debugger with breakpoints, step execution, memory dump, disassembly, and trace logging. Available in SDL mode.

Supports conditional breakpoints:
- CPU state: `--break 0x09F --condition "cpu.A == 0xFF"`
- VDC state: `--break 0x127 --condition "vdc.display_enabled == true"`
- Memory state: `--break 0x500 --condition "memory.external_ram[0x20] > 0x80"`

### disasm_tool (standalone)

MCS-48 ROM disassembler with automatic label generation.

```
./build/dev-mingw/disasm_tool.exe "roms/Satellite Attack (1981)(Philips)(EU).bin" --rom
./build/dev-mingw/disasm_tool.exe "roms/BIOS/bios_O2rom.bin"
```

Output includes labeled addresses, jump target resolution, and full disassembly. Pipe to a file for reference:

```
./build/dev-mingw/disasm_tool.exe "roms/BIOS/bios_O2rom.bin" > debug/bios_disasm.txt
```

## Debugging Workflow

1. **Reproduce** — use `--headless --frames N --trace` to capture state around the problem. Check screenshots to see what's on screen.
2. **Disassemble** — if the issue involves ROM behavior, disassemble the relevant ROM/BIOS with `disasm_tool`
3. **Cross-reference** — find the ROM handler for the feature (interrupt vectors at 0x003/0x007, cartridge entry at 0x400, BIOS routines like keyboard at 0x0B0, select_game at 0x2C3) and trace what it actually does
4. **Fix the emulator** — match the emulator's behavior to what the ROM expects
5. **Verify** — re-run the trace and confirm the state changes match expectations

When a game is "frozen", run headless to get snapshots and trace. Get ROM/BIOS disassembly. Find where the CPU is stuck in a loop, and trace back why it's not getting out — is it waiting for a register value that never arrives? An interrupt that never fires? A beam position that's never reached?

## Key Addresses

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
