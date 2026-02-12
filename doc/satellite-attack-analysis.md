# Satellite Attack Character Analysis

## Character Codes for Satellites

Based on the Satellite Attack disassembly (`doc/satellite-attack-disassembly.txt` lines 874-877):

```assembly
0729: [ BD 29 ] MOV R5, 29        ; 0x29 (char 'x') to R5
072B: [ 52 2F ] JB2, 2F            ; bit 2 of A set? jmp ret
072D: [ BD 10 ] MOV R5, 10        ; 0x10 (char '+') to R5
```

The game alternates between two characters based on bit 2 of internal RAM location 0x25:

- **Character 16 (0x10)**: '+' (plus sign)
  - ROM data: `0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00`
  - Visual pattern: vertical and horizontal bars forming a plus
  - Used when bit 2 of int_25 is CLEAR

- **Character 41 (0x29)**: 'x' (multiply symbol)
  - ROM data: `0x00,0x66,0x3C,0x18,0x3C,0x66,0x00,0x00`
  - Visual pattern: diagonal lines forming an X
  - Used when bit 2 of int_25 is SET

## Game Behavior

According to the disassembly (`doc/satellite-attack-disassembly.txt` line 151):
- The game sets R6 = 0x0C (12 decimal) - this is the number of satellites/characters
- Characters are written to VDC registers 0x10-0x3F (12 characters × 4 bytes each)
- Satellites alternate between '+' and 'X' patterns

## Character Rendering System

Characters in the Odyssey 2 are rendered using:
1. **Position**: Y (byte 0) and X (byte 1) coordinates
2. **Pattern Pointer**: 9-bit value (byte 2 bits 0-7, byte 3 bit 0) pointing into character ROM
3. **Color**: 3-bit color value (byte 3 bits 1-3)

The ROM address formula is: `(char_ptr + (char_y / 2) + char_row) & 0x1FF`

## Current Issue

The character rendering fix spec (`.kiro/specs/character-rendering-fix/`) addresses the bug where:
- Game correctly writes character data to VDC registers
- Display enable bit is set (foreground ON)
- But satellites (characters) are not appearing on screen

The spec provides comprehensive requirements for fixing the character rendering system to match hardware behavior.
