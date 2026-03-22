# Ship Movement Glitch Investigation

## Setup

- Game: Satellite Attack (1981)(Philips)(EU).bin
- BIOS: bios_O2rom.bin
- Region: USA (NTSC 60Hz)
- Input: press key 1 at frame 3, joystick 2 UP from frame 10 for 120 frames, DOWN from 130 for 240 frames
- o2em does NOT exhibit this glitch with the same game

## Reproduction

```bash
python run_emulator.py hl
```

Screenshots in `screenshots/`, trace in `trace_vdc.log`.

## Observed behavior (red ship sprite only)

```
F229-231: y=119-126 (8 rows) — normal, moving down 1px/frame
F232:     y=122-128 (7 rows) — missing 1 bottom row
F233:     y=123-128 (6 rows) — missing 2 bottom rows
F234-237: progressively losing rows, bottom stuck at ~128-130
F240-241: 1 row only
F242-243: ship completely gone
F244:     y=134-136 (3 rows) — reappears partial
F245:     y=135-142 (8 rows) — full ship
F246-254: y=137-142 (6 rows) — ship stuck, only 6 rows
F255-257: y=137-142 + stray red line at y=152
F258-259: y=137-142 + stray lines at y=152, 154, 155
F260:     y=150-157 (8 rows) — ship jumps to y=150, full again
```

## Findings

### VDC register writes are correct

All sprite 0 Y register writes happen at beam Y=246 (VBlank). Values increment
by 1 each frame. Writes are during VBlank as expected.

### Display is disabled mid-frame

The game disables display (writes 0x80 to control register 0xA0) at variable
scanlines during the visible area:

```
F229: display off at y=175
F230: display off at y=180
F231: display off at y=129
F232: display off at y=128
F233: display off at y=127
```

The game re-enables display ~27 scanlines later (after updating score registers).

### Game code flow (from disassembly)

```
0500: main_loop2 — game logic (variable duration)
051F: DIS I — disable interrupts
0520: CALL wait_until_scanline — waits for beam X >= 217 (HBlank)
0522: CALL set_VDC_control_register_to_0x80 — display OFF
0524: CALL update_status_bar — updates score quads (~27 scanlines of CPU time)
      (update_status_bar re-enables display at the end via JMP set_VDC_control_register_to_0xA8)
0526: CALL waitvsync — wait for VBlank
```

### Unexplained: stray scanlines at y=152

Frames 255-259 show the ship at y=137-142 (6 rows) plus a stray red line at
y=152. This is 10 scanlines below the ship body. The ship hasn't moved there
yet (it jumps to y=150 at frame 260). This cannot be explained by display
on/off timing — something is rendering a sprite row at the wrong Y position.

## Current investigation state (paused)

### What we know for certain

1. Sprite 0 Y register writes are correct: all during VBlank (y=246), incrementing by 1 each frame
2. Latched value at y=20 matches live value and increments correctly each frame
3. The game uses 3 sprites: sprite 0 (red ship), sprite 1 (blue satellite), sprite 2 (blue satellite)
4. The game disables display mid-frame (variable scanline 123-180) for ~27 scanlines to update score registers
5. The framebuffer is NOT cleared between frames — scanlines where display is off retain previous frame's pixels

### What doesn't add up

For SS255 (screenshot frame 255):
- Latched sprite 0 Y at y=20 = 141
- Sprite 0 should render at y=141-156 (16 rows)
- Screenshot shows red at y=137-142 and y=152
- y=137-142 is ABOVE the sprite Y of 141 — the sprite appears 4 scanlines too high
- y=152 is within the sprite range (141-156) — this is correct

The 4-scanline offset between latched Y (141) and rendered position (137) is unexplained.

### Possible next steps

1. Trace the latched sprite 0 Y value at the exact scanline where red pixels appear (y=137)
   to see if the latch has a different value there than at y=20
2. Check if the latch is being updated mid-frame by something other than the scanline
   transition code
3. Compare the sprite rendering Y calculation in is_sprite_pixel_at with o2em's
   to check for an offset bug
4. Check if the 320x240 screenshot has a Y mapping issue (though height isn't doubled)

### Debug code status

src/vdc.cpp has debug traces that need to be removed before committing.
The doc/case-studies/ship-movement-glitch.md has been updated with findings.
