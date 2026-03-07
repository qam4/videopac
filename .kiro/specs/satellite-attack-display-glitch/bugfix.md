# Bugfix Requirements Document

## Introduction

The Satellite Attack game (ROM: vp_34.bin) exhibits two related display issues in the Odyssey 2 emulator: (1) ship movement speed inconsistency when pressing up/down for a sustained period, and (2) visual glitches on the ship and satellite characters during movement. Both issues stem from the emulator's VDC (8245) not properly modeling when character/sprite register values are sampled by the rendering hardware relative to CPU writes.

In real hardware, the 8245 VDC renders objects using internally latched register values that are only updated at defined points in the scanline cycle. The emulator currently reads `state_.registers[]` directly on every pixel tick in `render_current_pixel()` → `is_character_pixel_at()` / `is_sprite_pixel_at()`, meaning CPU writes to graphic registers (0x00-0x7F) take effect immediately mid-scanline. This causes visual tearing when the CPU updates a character's position registers while the VDC is actively rendering that scanline.

The speed inconsistency is a secondary effect: the `wait_until_scanline` loop (0x71a-0x723) reads the beam X position register (0xA4) and loops until X + 0x27 overflows. If the beam X value returned by the emulator is not cycle-accurate to the real hardware, the loop iteration count varies between frames, causing the game logic that follows (including ship movement) to execute with inconsistent timing.

## References

- #[[file:.kiro/steering/debugging.md]] — Emulator debugging methodology, available tools, and key addresses
- #[[file:doc/case-studies/satellite-attack.md]] — Existing frame-by-frame analysis of Satellite Attack
- #[[file:doc/hardware/8245.md]] — Intel 8245 VDC hardware documentation
- #[[file:doc/reference/o2doc.md]] — VDC register map, sprite/character/grid rendering

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN the CPU writes to character position registers (VDC 0x10-0x3F bytes 0-1) while the VDC is rendering the visible portion of a scanline THEN the system applies the new position values immediately to the current pixel being rendered, causing the character to appear partially at the old position and partially at the new position on the same scanline (visual tearing)

1.2 WHEN the CPU writes to sprite position registers (VDC 0x00-0x0F bytes 0-1) while the VDC is rendering the visible portion of a scanline THEN the system applies the new position values immediately, causing the sprite to appear glitched or torn on that scanline

1.3 WHEN the game's `wait_until_scanline` loop (0x71a-0x723) reads the beam X position register (0xA4) and the emulator returns a beam X value that does not match the real hardware's X counter at the exact CPU read cycle THEN the loop exits at an inconsistent horizontal position across frames, causing variable amounts of CPU time remaining for game logic execution

1.4 WHEN the ship is moving upward or downward for a sustained period THEN the system renders the ship at inconsistent speeds because the variable loop exit timing from 1.3 causes the per-frame game logic (including position updates) to execute with different amounts of available CPU cycles

1.5 WHEN the CPU writes to character pattern pointer or color registers (VDC 0x10-0x3F bytes 2-3) while the VDC is rendering the visible portion of a scanline THEN the system uses the new pattern/color values immediately for the current pixel, causing the character to display with mixed old and new pattern data on the same scanline

### Expected Behavior (Correct)

2.1 WHEN the CPU writes to character position registers (VDC 0x10-0x3F bytes 0-1) while the VDC is rendering the visible portion of a scanline THEN the system SHALL continue rendering the character at its previous position for the remainder of that scanline, and the new position SHALL only take effect starting from the next scanline's rendering pass

2.2 WHEN the CPU writes to sprite position registers (VDC 0x00-0x0F bytes 0-1) while the VDC is rendering the visible portion of a scanline THEN the system SHALL continue rendering the sprite at its previous position for the remainder of that scanline, and the new position SHALL only take effect starting from the next scanline's rendering pass

2.3 WHEN the game's `wait_until_scanline` loop reads the beam X position register (0xA4) THEN the system SHALL return a beam X value that is consistent with the master clock's scanline tick position at the exact point when the CPU's MOVX read strobe samples the bus, producing a deterministic loop iteration count for a given entry point within the scanline

2.4 WHEN the ship is moving upward or downward for a sustained period THEN the system SHALL render the ship moving at a constant speed because the deterministic beam X timing ensures consistent CPU cycle budgets per frame for game logic execution

2.5 WHEN the CPU writes to character pattern pointer or color registers (VDC 0x10-0x3F bytes 2-3) while the VDC is rendering the visible portion of a scanline THEN the system SHALL continue using the previous pattern/color values for the remainder of that scanline, and the new values SHALL only take effect starting from the next scanline's rendering pass

### Unchanged Behavior (Regression Prevention)

3.1 WHEN the CPU writes to graphic registers (0x00-0x7F) during HBLANK or VBLANK THEN the system SHALL CONTINUE TO apply those values before the next visible scanline begins rendering, as games rely on blanking-period updates being visible on the next scanline

3.2 WHEN the CPU writes to non-graphic VDC registers (control 0xA0, status 0xA1, collision 0xA2, color 0xA3, sound 0xA7-0xAA, grid 0xC0-0xE9) THEN the system SHALL CONTINUE TO apply those values with the same timing as before, since these registers are not part of the per-object rendering pipeline

3.3 WHEN the VDC renders characters and sprites during normal gameplay without mid-scanline register updates THEN the system SHALL CONTINUE TO render them at the correct positions, with correct patterns, colors, and priority ordering

3.4 WHEN the CPU reads the beam X position register (0xA4) during VBLANK or HBLANK THEN the system SHALL CONTINUE TO return the correct beam position value consistent with the master clock state

3.5 WHEN the CPU reads the VDC status register (0xA1) THEN the system SHALL CONTINUE TO return correct HBLANK, VBLANK, and collision status bits and clear the appropriate flags on read

3.6 WHEN games other than Satellite Attack write to VDC graphic registers during VBLANK (the standard update pattern) THEN the system SHALL CONTINUE TO display correctly since the scanline-latching mechanism is transparent when writes occur during blanking periods
