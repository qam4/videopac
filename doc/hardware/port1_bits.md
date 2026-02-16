# Port 1 Bit Definitions

Port 1 (P1) is an 8-bit I/O port used to control various hardware subsystems in the Videopac/Odyssey2.

## Bit Layout

```
Bit:  7      6       5      4       3       2       1       0
     P17    P16     P15    P14     P13     P12     P11     P10
     |      |       |      |       |       |       |       |
     |      |       |      |       |       |       |       +-- Bank select bit 0 (low)
     |      |       |      |       |       |       +---------- Bank select bit 1 (high)
     |      |       |      |       |       +------------------ Keyboard scan enable (active low)
     |      |       |      |       +-------------------------- VDC enable (active low)
     |      |       |      +---------------------------------- External RAM enable (active low)
     |      |       +----------------------------------------- Not connected
     |      +------------------------------------------------- Copy mode enable (active high)
     +-------------------------------------------------------- Luminance enable (active high)
```

## Bit Descriptions

### P10 (Bit 0) - Bank Select Low
- Used for ROM cartridge bank switching
- For 4KB cartridges: P10 selects between 2 banks
- For 8KB cartridges: P10 and P11 together select between 4 banks

### P11 (Bit 1) - Bank Select High
- Used for ROM cartridge bank switching
- Only used for 8KB cartridges (4 banks)
- Combined with P10 to form 2-bit bank selector

### P12 (Bit 2) - Keyboard Scan Enable
- **Active Low**: Set to 0 to enable keyboard scanner
- Set to 1 to disable keyboard scanner
- Must be enabled when reading keyboard via Port 2

### P13 (Bit 3) - VDC Enable
- **Active Low**: Set to 0 to enable VDC on external bus
- Set to 1 to disable VDC on external bus
- **VDC Writes**: Work with just P13=0 (regardless of P14 state)
- **VDC Reads**: Require P13=0 AND P14=1
- **For VDC access**: P13=0, P14=1 (reads and writes)
- **For RAM access**: P13=1, P14=0
- **For Copy mode**: P13=0, P14=0, P16=1
- **Hardware Note**: If P13=0 AND P14=0 (and P16=0), writes go to BOTH VDC and EXRAM simultaneously

### P14 (Bit 4) - External RAM Enable
- **Active Low**: Set to 0 to enable external RAM on bus
- Set to 1 to disable external RAM on bus
- **For VDC access**: P13=0, P14=1 (reads require this, writes don't)
- **For RAM access**: P13=1, P14=0
- **For Copy mode**: P13=0, P14=0, P16=1
- **Hardware Note**: If P13=0 AND P14=0 (and P16=0), writes go to BOTH VDC and EXRAM simultaneously

### P15 (Bit 5) - Not Connected
- This bit is not connected to any hardware
- Can be used for software flags

### P16 (Bit 6) - Copy Mode Enable
- **Active High**: Set to 1 to enable copy mode
- Copy mode allows efficient data transfer from RAM to VDC
- When enabled (with P13=0, P14=0):
  - External reads come from RAM
  - External writes go to VDC
- Used by BIOS for fast graphics updates

### P17 (Bit 7) - Luminance Enable
- **Active High**: Set to 1 to enable luminance output
- Controls luminance signal from VDC to video mixer
- Affects video output brightness/quality

## Common Port 1 Values

### BIOS Setup Routines

**Set up VDC Access** (address 0x0E7):
```
ORL P1, #0xBC    ; Set bits: P17, P15, P14, P13, P12 (0b10111100)
                 ; Result: !kbscan !vdcen !ramen lumen
ANL P1, #0xB7    ; Clear bits: P16, P13 (0b10110111)
                 ; Result: Enables VDC (P13=0), disables copy mode
```

**Set up RAM Access** (address 0x0EC):
```
ORL P1, #0xBC    ; Set bits: P17, P15, P14, P13, P12
ANL P1, #0xB7    ; Clear bits: P16, P13
                 ; Result: Enables RAM (P14=0), disables VDC
```

### Typical Configurations

| Configuration | P17 | P16 | P15 | P14 | P13 | P12 | P11 | P10 | Hex  | Description |
|---------------|-----|-----|-----|-----|-----|-----|-----|-----|------|-------------|
| VDC Access    |  1  |  0  |  x  |  1  |  0  |  1  |  x  |  x  | 0x9x | VDC enabled, RAM disabled |
| RAM Access    |  1  |  0  |  x  |  0  |  1  |  1  |  x  |  x  | 0x8x | RAM enabled, VDC disabled |
| Copy Mode     |  1  |  1  |  x  |  0  |  0  |  1  |  x  |  x  | 0xCx | Copy RAM to VDC |
| Keyboard Scan |  1  |  0  |  x  |  1  |  1  |  0  |  x  |  x  | 0x9x | Keyboard enabled |

## Notes

1. **P13 and P14 behavior differs for reads vs writes**:
   - **VDC Writes**: Work with just P13=0 (P14 state doesn't matter, as long as P16=0)
   - **VDC Reads**: Require both P13=0 AND P14=1
   - **Simultaneous write**: If P13=0 AND P14=0 (and P16=0), data writes to BOTH VDC and EXRAM
   - This asymmetry is a hardware characteristic confirmed through testing and internet research

2. **P13 and P14 typical configurations**:
   - VDC access: P13=0, P14=1 (works for both reads and writes)
   - RAM access: P13=1, P14=0
   - Copy mode: P13=0, P14=0, P16=1 (special case)

3. **Active Low vs Active High**:
   - P10-P15: Active low (0 = enabled)
   - P16-P17: Active high (1 = enabled)

4. **BIOS Conventions**:
   - BIOS typically keeps P17 (luminance) set to 1
   - BIOS typically keeps P12 (keyboard scan) set to 1 except when scanning
   - Bank select bits (P10, P11) are managed by cartridge code

## BIOS Naming Convention

The BIOS disassembly uses these signal names (from doc/french_bios_annotated.txt):

| BIOS Name | Port 1 Bit | Description | Active |
|-----------|------------|-------------|--------|
| `!kbscan` | P12 (bit 2) | Keyboard scan enable | Low (!) |
| `!vdcen`  | P13 (bit 3) | VDC enable | Low (!) |
| `!ramen`  | P14 (bit 4) | External RAM enable | Low (!) |
| `copyen`  | P16 (bit 6) | Copy mode enable | High |
| `lumen`   | P17 (bit 7) | Luminance enable | High |

**Naming Convention**:
- `!` prefix indicates active-low signal (0 = enabled)
- `en` suffix means "enable"
- No `!` prefix means active-high signal (1 = enabled)

**Example from BIOS**:
```
ORL P1, #0xBC    ; set : !kbscan !vdcen !ramen lumen
ANL P1, #0xB7    ; clear : !vdcen copyen
```
This sets P12=1, P13=1, P14=1, P17=1, then clears P13=0, P16=0, resulting in VDC enabled.

## References

- doc/o2doc.md section 1.1 "I/O port 1"
- doc/french_bios_annotated.txt (BIOS setup routines at 0x0E7, 0x0EC)
- Hardware behavior confirmed via testing and internet research


---

# Port 2 Bit Definitions

Port 2 (P2) is an 8-bit I/O port used primarily for keyboard and joystick input.

## Bit Layout

```
Bit:  7      6       5      4       3       2       1       0
     P27    P26     P25    P24     P23     P22     P21     P20
     |      |       |      |       |       |       |       |
     |      |       |      |       |       |       |       +-- Keyboard row select bit 0 (W)
     |      |       |      |       |       |       +---------- Keyboard row select bit 1 (W)
     |      |       |      |       |       +------------------ Keyboard row select bit 2 (W)
     |      |       |      |       +-------------------------- Unused
     |      |       |      +---------------------------------- Key press indicator (R)
     |      |       +----------------------------------------- Keyboard column bit 0 (R)
     |      +------------------------------------------------- Keyboard column bit 1 (R)
     +-------------------------------------------------------- Keyboard column bit 2 (R)
```

## Bit Descriptions

### P20-P22 (Bits 0-2) - Keyboard Row Select (Write)
- **Write Only**: Select which keyboard row to scan
- Values 0-5 select keyboard rows 0-5
- Value 0 also enables joystick 2 on data bus
- Value 1 also enables joystick 1 on data bus
- Reference: doc/o2doc.md section 1.2 "I/O port 2"

### P23 (Bit 3) - Unused
- This bit is not used

### P24 (Bit 4) - Key Press Indicator (Read)
- **Read Only**: Indicates if a key is pressed
- 0 = Key pressed on currently selected row
- 1 = No key pressed
- Reference: doc/o2doc.md section 1.2 "I/O port 2"

### P25-P27 (Bits 5-7) - Keyboard Column Read (Read)
- **Read Only**: Column number of pressed key
- Only valid when P24 = 0 (key pressed)
- Values 0-7 indicate which column has the pressed key
- Reference: doc/o2doc.md section 1.2 "I/O port 2"

## Keyboard Matrix

The keyboard is organized as a 6-row by 8-column matrix:

| Row (P20-P22) | Keys |
|---------------|------|
| 0 (0b000) | 0, 1, 2, 3, 4, 5, 6, 7 |
| 1 (0b001) | 8, 9, -, +, *, /, =, Yes |
| 2 (0b010) | Q, W, E, R, T, Y, U, I |
| 3 (0b011) | A, S, D, F, G, H, J, K |
| 4 (0b100) | Z, X, C, V, B, N, M, . |
| 5 (0b101) | Space, ?, L, P, O, Clear, Enter, No |

## Joystick Reading

- Write 0 to P20-P22: Enables joystick 2 on data bus
- Write 1 to P20-P22: Enables joystick 1 on data bus
- Use `INS A, BUS` instruction to read joystick state
- Reference: doc/o2doc.md section 5.0 "Joysticks"

## Usage Notes

1. **P20-P23 are dual-purpose**: They are also used as the upper 4 bits of the address bus
2. **BIOS requirement**: Because of the dual-purpose nature, keyboard reading should be done through BIOS routines
3. **Keyboard scanning sequence**:
   - Disable VDC and RAM (set P13=1, P14=1)
   - Enable keyboard scanner (set P12=0)
   - Write row number to P20-P22
   - Read P24 to check if key pressed
   - If P24=0, read P25-P27 to get column number
4. **P12 must be low**: Port 1 bit 2 (P12) must be set to 0 to enable keyboard scanning

## BIOS Keyboard Routine

The BIOS provides a keyboard scanning routine at address 0x0B0:
- Scans all 6 rows
- Debounces key presses
- Returns key code in R7 (with bit 7 clear if valid key)
- Reference: doc/french_bios_annotated.txt, address 0x0B0

## References

- doc/o2doc.md section 1.2 "I/O port 2"
- doc/french_bios_annotated.txt (Keyboard routine at 0x0B0)
- include/input.h (Keyboard matrix implementation)
