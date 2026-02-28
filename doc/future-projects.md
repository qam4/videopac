# Future Emulator Projects

Ideas for next emulator projects, building on the videopac architecture
(CPU/VDC/Memory component separation, libretro core, SDL frontend, debugger, CI/CD).

## Thomson MO5 (recommended next)

French 8-bit home computer (1984). Personal connection — owned one.

### Hardware
- **CPU**: Motorola 6809 at 1 MHz — elegant instruction set, two index registers,
  two stack pointers, proper 16-bit ops, position-independent code support
- **RAM**: 48KB (32KB user + 16KB video)
- **Video**: "forme/fond" system — 320x200, 8x1 attribute cells, each cell has
  foreground/background color pair from 16 colors. Unique to Thomson machines.
- **Sound**: 1-bit buzzer (much simpler than Videopac shift register audio)
- **Storage**: Cassette interface
- **Input**: Chiclet keyboard, light pen (optional)

### Why this one
- Manageable complexity, similar scope to Videopac
- 6809 is a joy to implement — arguably the best 8-bit CPU ever designed
- Underserved in the libretro ecosystem (no great core exists)
- French documentation and community are strong (DCMOTO, French retro forums)
- Architecture maps 1:1 from videopac: swap CPU (8048→6809), VDC (8245→Thomson video), memory map
- Libretro core, debugger, CI/CD, save states, SDL frontend all carry over

### Resources
- **Reference emulator**: [DCMO5](https://github.com/pulkomandy/dcmo5) (GPLv3) — same role as o2em was for videopac
- **Libretro reference**: [lr-theodore](https://github.com/Music-Maniacs/theodore) — existing Thomson MO/TO libretro core (based on DCMO5)
- **MESS/MAME driver**: [MO5 driver notes](https://mine.perso.lip6.fr/mess/mo5.html.en) — detailed memory map, I/O, video constraints, cassette format
- **Hardware wiki**: [Pulkomandy's "DON'T PANIC"](http://pulkomandy.tk/wiki/doku.php?id=documentations:hardware:mo5) — hardware docs aimed at demoscene coders
- **6809 CPU reference**: Darren Atkinson's "Motorola 6809 and Hitachi 6309 Programming Reference" — complete opcode tables, addressing modes, cycle counts
- **6809 CPU datasheet**: Motorola MC6809E datasheet (the E variant is what the MO5 uses — externally clocked)
- **Dragon/CoCo docs**: [6809.org.uk](https://www.6809.org.uk/dragon/hardware.shtml) — Dragon 32 uses same CPU, good 6809 hardware details
- **MO5 BIOS**: 16KB ROM (4KB monitor + 12KB BASIC 1.0) — needed to boot, not freely distributable

### Video details (EFGJ03L gate array)
- NOT a standard MC6847 — Thomson custom gate array
- 320×200 bitmap, 16 fixed colors
- "forme/fond" (shape/background): each 8-pixel block has a foreground+background color pair
- Video RAM: 8KB pixel data (0x0000-0x1FFF) + 8KB color data (0x2000-0x3FFF)
- No sprites, no text mode, no scrolling hardware — all software-rendered
- Simpler than the 8245 VDC (no collision detection, no character ROM, no grid system)

### MO5 memory map
- 0x0000-0x1FFF: Video RAM (pixel data, 8KB)
- 0x2000-0x3FFF: Video RAM (color attributes, 8KB)
- 0x4000-0x5FFF: User RAM (8KB)
- 0x6000-0x9FFF: User RAM (16KB) or cartridge
- 0xA000-0xA7FF: I/O space (PIA 6821, gate array registers)
- 0xA800-0xBFFF: Reserved
- 0xC000-0xEFFF: BASIC ROM (12KB)
- 0xF000-0xFFFF: Monitor ROM (4KB, includes reset/interrupt vectors)

### Development milestones
1. Get 6809 CPU passing instruction tests (many test ROMs available — CoCo/Dragon community)
2. Implement memory map + PIA (6821) for basic I/O
3. Implement video gate array (bitmap rendering)
4. Boot MO5 BIOS to BASIC prompt — first real milestone
5. Cassette loading (K7 format)
6. Keyboard input
7. Light pen emulation (the crayon optique)
8. Libretro core + Android

---

## Amstrad CPC 6128

### Hardware
- **CPU**: Z80
- **Video**: CRTC 6845 + gate array
- **Sound**: AY-3-8910

### Notes
- CRTC tricks (rupture, overscan) make this moderate-to-hard
- Z80 experience opens the door to ColecoVision, SG-1000, MSX, SMS
- Mature existing emulators (WinAPE, Caprice32)

---

## Amiga 500

### Hardware
- **CPU**: Motorola 68000
- **Custom chips**: Agnus (DMA/copper/blitter), Denise (video), Paula (audio/disk)

### Notes
- Serious undertaking — custom chips are deeply interleaved, bus cycle accuracy matters
- 68000 alone is a big CPU
- Existing emulators (WinUAE, FS-UAE) took decades
- Save for later, after a couple more emulators under the belt

---

## What carries over from videopac
- Build system (CMake), CI/CD (GitHub Actions), release workflow
- Libretro core structure (swap EmulatorCore internals)
- Debugger UI (ImGui), save state framework
- SDL frontend, input mapping
- Test infrastructure (GTest, RapidCheck)
