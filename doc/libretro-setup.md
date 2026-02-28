# Libretro Core Setup

## RetroArch on Android

1. Download `videopac-android-arm64.zip` from the [releases page](https://github.com/qam4/videopac/releases)
2. Extract `videopac_libretro.so`
3. In RetroArch, go to **Load Core → Install or Restore a Core** and select the `.so` file
   - Or manually copy it to RetroArch's `cores/` directory
4. (Optional) Copy `videopac_libretro.info` next to the `.so` — this provides
   display name and BIOS info in the RetroArch UI, but isn't required to run
5. Place the BIOS file in RetroArch's `system/` directory (any of these filenames work):
   - `o2rom.bin` (preferred)
   - `bios_O2rom.bin`
   - `odyssey2.bin`
   - `c52.bin` / `bios_c52.bin` (French C52)
   - `g7400.bin` / `bios_g7400.bin` (Videopac+)
   - `jopac.bin` / `bios_jopac.bin` (French Videopac+)
6. Load a ROM file (`.bin` or `.rom`) through RetroArch, selecting the videopac core

## RetroArch on Desktop (Linux/Windows/macOS)

Same steps, using the platform-specific zip from the release.
The libretro core is in the `lib/` directory of the archive.

## Controls

| RetroArch Button | Videopac Function |
|-----------------|-------------------|
| D-Pad           | Joystick          |
| A               | Fire              |
| B               | Key 0 (game select) |
| Y               | Key 1             |
| X               | Key 2             |
| L               | Key 3             |
| Select          | Space             |
| Start           | Enter             |

Player 2 uses the same layout on controller port 2.

## Core Options

- **Region**: NTSC (60Hz) or PAL (50Hz)
- **Palette**: Standard (Odyssey 2 / Videopac) or Videopac+ (G7400)

## Typical Usage

Most games require pressing a number key to select the game variant, then Enter to start:
1. Load the ROM
2. Press **B** (Key 0) to select game variant, or **Y/X/L** for variants 1/2/3
3. Press **Start** (Enter) to begin
