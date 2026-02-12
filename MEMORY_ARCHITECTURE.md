# Videopac/Odyssey 2 Memory Architecture

## CPU Internal RAM (64 bytes)
The 8048 CPU has 64 bytes of RAM built into the chip:

```
0x00-0x07: Register Bank 0 (R0-R7) - selected by SEL RB0
0x08-0x17: Stack (8 levels) and scratch pad
0x18-0x1F: Scratch pad memory
0x20-0x3F: General purpose RAM (32 bytes)
```

**Register Banks:**
- The CPU has TWO sets of 8 registers (R0-R7)
- Bank 0: addresses 0x00-0x07 (default)
- Bank 1: addresses 0x18-0x1F (24-31 decimal)
- Selected with `SEL RB0` or `SEL RB1` instructions
- Used for interrupt handling (main program uses one bank, interrupts use the other)

**Access:** Direct CPU instructions (MOV, INC, DEC, etc.)

## External RAM (128 bytes)
Separate RAM chip outside the CPU:

```
0x00-0x7F: 128 bytes of external RAM
```

**Access:** 
- MOVX instruction (external memory access)
- Controlled by Port 1 bit P14 (active low)
- When P14=0 and P13=1: External RAM is accessible

**Purpose:**
- General purpose storage
- Temporary buffers
- BIOS uses addresses 0x7F, 0x7E, 0x7D... for VDC copy descriptors

## VDC Registers (256 bytes address space)
Video Display Controller registers:

```
0x00-0x0F: Internal VDC control registers
0x10-0x1F: Sprite registers (4 sprites × 4 bytes each)
           0x10-0x13: Sprite 0 (X, Y, Color, Pattern)
           0x14-0x17: Sprite 1
           0x18-0x1B: Sprite 2
           0x1C-0x1F: Sprite 3
0x20-0x7F: Character registers (12 characters × 4 bytes each)
           Each character: X, Y, Color, Pattern
0x80-0x8F: Quad shape data (4 quads × 4 bytes each)
0x90-0x97: Sprite shape data (8 bytes)
0x98-0x9F: Character shape data
0xA0-0xA7: VDC control/status registers
```

**Access:**
- MOVX instruction (external memory access)
- Controlled by Port 1 bit P13 (active low)
- When P13=0: VDC registers are accessible

## Port 1 Control Signals

Port 1 controls which external device is accessed by MOVX instructions:

```
P13 (0x08): VDC enable (active low)  - 0=VDC accessible
P14 (0x10): RAM enable (active low)  - 0=RAM accessible
P16 (0x40): Copy mode (active high)  - 1=copy mode
```

**Access Modes:**

1. **VDC only:** P13=0, P14=1
   - MOVX reads/writes go to VDC registers

2. **RAM only:** P13=1, P14=0
   - MOVX reads/writes go to external RAM

3. **Both VDC and RAM:** P13=0, P14=0
   - MOVX writes go to BOTH VDC and RAM simultaneously
   - MOVX reads come from VDC

4. **Copy mode:** P13=0, P14=0, P16=1
   - MOVX reads come from RAM
   - MOVX writes go to VDC only (RAM writes disabled)
   - Used by BIOS to copy data from RAM to VDC during VBLANK

## BIOS Copy Mechanism

The BIOS uses a clever mechanism to copy data from external RAM to VDC during VBLANK:

**Setup (done by game code):**
1. Write data to external RAM addresses (backwards from 0x7D, 0x7C, 0x7B...)
2. Write VDC target address to RAM[0x7E]
3. Write byte count to RAM[0x7F]
4. Set bit 7 of internal RAM location 0x3F to request copy

**Execution (done by BIOS during VBLANK):**
1. BIOS reads RAM[0x7F] to get byte count
2. BIOS reads RAM[0x7E] to get VDC target address
3. BIOS enables copy mode (P16=1, P13=0, P14=0)
4. BIOS loops:
   - Read from RAM[0x7D], RAM[0x7C], RAM[0x7B]... (backwards)
   - Write to VDC[target], VDC[target+1], VDC[target+2]... (forwards)

## Current Issue

In frame 6, the game is setting up a copy with:
- RAM[0x7F] = 0xF8 (248 bytes)
- RAM[0x7E] = 0xF8 (VDC address 0xF8)

This is copying 248 bytes to VDC starting at address 0xF8, which is beyond the valid VDC register range (0x00-0xA7). This means the sprite and character data is being written to invalid VDC addresses and won't be displayed.
