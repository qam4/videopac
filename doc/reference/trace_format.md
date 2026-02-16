# Trace Log Format Documentation

## Overview

The emulator can generate detailed execution traces when the debugger trace is enabled. The trace logs CPU instruction execution along with system state snapshots.

## Trace Frequency

**Important**: Traces are logged once per CPU instruction execution, NOT per master clock cycle.

- VDC runs at ~3.54 MHz (VDC clock)
- CPU runs at ~358 kHz (~10x slower than VDC)
- Each CPU instruction takes multiple cycles (typically 1-2 machine cycles, where 1 machine cycle = ~10 VDC cycles)

This means:
- Between each trace line, the VDC has advanced approximately 10-20 cycles
- The VDC state shown is a snapshot at the moment the CPU instruction completes
- VDC beam position advances significantly between trace lines

## Trace Line Format

```
[F:<frame> C:<cycle>] <address>: <opcode> <instruction> | <cpu_state> <port_state> <vdc_state> [<operation_details>]
```

### Example Trace Line

```
[F:5 C:297419] 0x171: 90     MOVX @R0,A | A=0xf8 PSW=0x90 P1=0x1c P2=0x00 [VDC:OFF RAM:OFF] RB0 VDC[beam:123,5 ctrl:0x00 stat:0x02 disp:OFF vbl:N] [WRITE @R0=0x10 val=0xf8 -> NOWHERE!]
```

## Field Descriptions

### Frame and Cycle Counter
- `[F:<frame> C:<cycle>]` - Frame number and master clock cycle count
  - Frame: Current video frame number (increments at VBLANK)
  - Cycle: Master clock cycle count (VDC clock cycles, NOT CPU cycles)

### Instruction Information
- `<address>: <opcode> <instruction>` - PC address, raw opcode bytes, disassembled instruction

### CPU State
- `A=0x__` - Accumulator value
- `PSW=0x__` - Program Status Word
  - Bit 7: Carry flag (C)
  - Bit 6: Auxiliary Carry (AC)
  - Bit 5: F0 flag
  - Bit 4: Register Bank Select (BS)

### Port State
- `P1=0x__` - Port 1 value (I/O control port)
- `P2=0x__` - Port 2 value (keyboard/input port)

### Port 1 Decoded Control Signals
- `[VDC:ON/OFF RAM:ON/OFF COPY?]`
  - VDC: ON when P13=0 (active low), enables VDC access
  - RAM: ON when P14=0 (active low), enables external RAM access
  - COPY: Present when P16=1, enables VDC copy mode

### Register Bank
- `RB0` or `RB1` - Current register bank (0 or 1)

### VDC State (Snapshot)
- `VDC[beam:<x>,<y> ctrl:0x__ stat:0x__ disp:ON/OFF vbl:Y/N]`
  - `beam:<x>,<y>` - Current beam position (horizontal, vertical)
    - X: 0-226 (227 cycles per scanline including HBLANK)
    - Y: 0-261 (NTSC) or 0-311 (PAL)
  - `ctrl:0x__` - VDC control register (0xA0)
  - `stat:0x__` - VDC status register (0xA1)
  - `disp:ON/OFF` - Display enabled (bit 5 of control register)
  - `vbl:Y/N` - VBLANK active (Y >= 240 for NTSC, Y >= 288 for PAL)

**Note**: This VDC state is a snapshot at the moment the CPU instruction completes. The VDC has advanced ~10 cycles since the previous trace line.

### Operation Details (for specific instructions)

#### MOVX @Rr,A (Write to external memory)
```
[WRITE @R0=0x10 val=0xf8 -> VDC+RAM]
```
- Register used as address pointer
- Address value
- Value being written
- Destination based on Port 1:
  - `-> VDC+RAM` - Both VDC and RAM enabled (normal write)
  - `-> VDC` - Only VDC enabled (copy mode or RAM disabled)
  - `-> RAM` - Only RAM enabled (VDC disabled)
  - `-> NOWHERE!` - Both VDC and RAM disabled (write is lost!)

#### MOVX A,@Rr (Read from external memory)
```
[READ @R0=0x10 <- VDC]
```
- Register used as address pointer
- Address value
- Source based on Port 1:
  - `<- VDC` - Reading from VDC registers
  - `<- RAM` - Reading from external RAM
  - `<- 0xFF` - Both disabled (returns 0xFF)

#### OUTL P1,A (Port 1 output)
```
[P1 CHANGE: 0x1c -> 0x04]
```
- Shows old Port 1 value -> new Port 1 value

#### OUTL P2,A (Port 2 output)
```
[P2 CHANGE: 0x00 -> 0xff]
```
- Shows old Port 2 value -> new Port 2 value

#### Register Operations (MOV, INC, DEC)
```
[R0=0x10]
```
- Shows register value after operation

## Common Debugging Scenarios

### Checking VDC Access
Look for Port 1 state when MOVX instructions execute:
- `[VDC:ON RAM:ON]` - Normal VDC write (both enabled)
- `[VDC:OFF RAM:OFF]` - **BUG**: Writes go nowhere!

### Tracking Beam Position
The VDC beam position shows where the electron beam is on the screen:
- Visible area: X: 0-159, Y: 0-239 (NTSC) or 0-287 (PAL)
- HBLANK: X: 160-226
- VBLANK: Y: 240-261 (NTSC) or 288-311 (PAL)

### Finding Character Rendering Issues
1. Look for MOVX writes to addresses 0x10-0x3F (character registers)
2. Check Port 1 state - VDC must be enabled
3. Check VDC beam position - writes during visible area may cause artifacts
4. Check display enabled flag

## Enabling Trace Logging

Trace logging is controlled by the debugger. To enable:

```cpp
debugger->enable_trace(true);
```

Or use the `--debug` command-line flag when running the emulator.

## Performance Considerations

Trace logging generates large amounts of data:
- ~1000-2000 CPU instructions per frame
- ~60 frames per second
- ~60,000-120,000 trace lines per second

For debugging specific issues, consider:
1. Running in headless mode with limited frames (`--headless --frames 10`)
2. Using breakpoints to stop at specific addresses
3. Filtering trace output to specific frame ranges
