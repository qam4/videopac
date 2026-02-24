# VDC Character Encoding

## Overview

The Intel 8245 VDC has an internal character ROM with 64 characters (0x00-0x3F). Each character is defined by 8 bytes representing an 8x8 pixel pattern.

## Character Map

Based on the character ROM definition in `src/vdc.cpp`:

### Digits (0x00-0x09)
- 0x00: '0'
- 0x01: '1'
- 0x02: '2'
- 0x03: '3'
- 0x04: '4'
- 0x05: '5'
- 0x06: '6'
- 0x07: '7'
- 0x08: '8'
- 0x09: '9'

### Punctuation (0x0A-0x0D)
- 0x0A: ':'
- 0x0B: special symbol
- 0x0C: (blank/space)
- 0x0D: '?'

### Letters (0x0E-0x2D)
- 0x0E: 'L'
- 0x0F: 'P'
- 0x10: '+'
- 0x11: 'W'
- 0x12: 'E'
- 0x13: 'R'
- 0x14: 'T'
- 0x15: 'U'
- 0x16: 'I'
- 0x17: 'O'
- 0x18: 'Q'
- 0x19: 'S'
- 0x1A: 'D'
- 0x1B: 'F'
- 0x1C: 'G'
- 0x1D: 'H'
- 0x1E: 'J'
- 0x1F: 'K'
- 0x20: 'A'
- 0x21: 'Z'
- 0x22: 'X'
- 0x23: 'C'
- 0x24: 'V'
- 0x25: 'B'
- 0x26: 'M'
- 0x27: '.'
- 0x28: '-'
- 0x29: 'x' (multiply)
- 0x2A: '÷' (divide)
- 0x2B: '='
- 0x2C: 'Y'
- 0x2D: 'N'

### Special Characters (0x2E-0x3F)
- 0x2E: '/'
- 0x2F: block (solid)
- 0x30: '10' (special)
- 0x31: ball
- 0x32: man right
- 0x33: man right walk
- 0x34: man left walk
- 0x35: man left
- 0x36: arrow right
- 0x37: tree
- 0x38: slope left
- 0x39: slope right
- 0x3A: man forward
- 0x3B: '\'
- 0x3C: ship 1
- 0x3D: plane
- 0x3E: ship 2
- 0x3F: ship 3

## Example: "KILLER BEES"

To display "KILLER BEES" using VDC characters:

```
K I L L E R   B E E S
1F 16 0E 0E 12 13 0C 25 12 12 19
```

## BIOS vs VDC Encoding

**Important**: The BIOS uses a DIFFERENT encoding for text strings stored in ROM!

### BIOS Text Encoding
The BIOS stores text using instruction bytes that encode characters. Example from 0x02f2 ("SELECT GAME"):
- S = 0x19 (INC R1)
- E = 0x12 (JB0)
- L = 0x0E (JB0)
- etc.

### VDC Character Encoding
The VDC character ROM uses the mapping shown above (K=0x1F, I=0x16, etc.)

### Why Two Encodings?
- BIOS text is stored as instruction bytes to save space
- VDC character codes are used when writing to VDC registers
- BIOS routines convert between the two encodings when displaying text

## Usage in Games

Games typically:
1. Store text as BIOS-encoded instruction bytes in ROM
2. Call BIOS display routines (e.g., 0x3EA)
3. BIOS converts to VDC character codes
4. BIOS writes character patterns to VDC registers

This is why searching for "KILLER BEES" as VDC character codes in ROM fails - the text is stored using BIOS encoding and converted at runtime.

## References

- Character ROM definition: `src/vdc.cpp` (character_rom_ array)
- BIOS text example: `doc/o2romsrc.txt` at 0x02f2
- VDC documentation: `doc/hardware/8245.md`
