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
- DCMOTO emulator and documentation
- MO5 technical manual
- French retro computing forums

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
