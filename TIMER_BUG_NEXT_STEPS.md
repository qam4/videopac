# Timer Bug - Next Steps

## Key Discovery
The corruption happens at frame 56, which is exactly when the timer first counts down from "02:00" to "01:59". This is the FIRST time all 4 digits change simultaneously:
- Minutes high: 0 → 0 (no change)
- Minutes low: 2 → 1 (changes)
- Seconds high: 0 → 5 (changes)  
- Seconds low: 0 → 9 (changes)

## Timer Display Mechanism

### Data Structure
The BIOS prepares 8 bytes in RAM (0x7D-0x7A, backwards) to be copied to VDC quad registers starting at 0x42:
- Byte 0 (RAM[0x7D]): Character data
- Byte 1 (RAM[0x7C]): X position (0x02)
- Byte 2 (RAM[0x7B]): Y position (0x52)
- Byte 3 (RAM[0x7A]): Character data
- ... (continues for all 4 digits)

### Copy Process
1. BIOS up_down_counter (0x1B0) updates RAM[1] and RAM[2] with new BCD values
2. BIOS prepares display data in RAM[0x7D-0x7A]
3. Sets RAM[0x7E] = 0x42 (VDC target address - quad registers)
4. Sets RAM[0x7F] = 0x08 (byte count)
5. Sets RAM[0x3F] bit 7 to request copy
6. VBLANK interrupt handler calls copying_code (0x0A3)
7. Copies 8 bytes from RAM to VDC quad registers

## Hypothesis

The bug occurs during the RAM-to-VDC copy when:
1. All 4 quad character registers need updating simultaneously
2. The copy happens during VBLANK
3. Something goes wrong with the copy mechanism

Possible causes:
1. **Copy mode not enabled correctly**: P16=1, P13=0, P14=0 might not be set properly
2. **VDC address auto-increment issue**: After writing to 0x42, address should increment to 0x43, 0x44, etc.
3. **Timing issue**: Copy might start too late or be interrupted
4. **Register corruption**: The quad registers might be getting corrupted data

## Recommended Investigation Steps

### Step 1: Add Logging to RAM-to-VDC Copy
Add detailed logging in `src/memory.cpp` when copy mode is active:
```cpp
if (copy_mode() && vdc_enabled() && ram_enabled()) {
    // Log: "COPY MODE WRITE: VDC[0x{address:02x}] = 0x{value:02x} from RAM"
    if (vdc_) {
        vdc_->write_register(address, value);
    }
    return;
}
```

### Step 2: Log RAM[0x3F] Bit 7 Changes
Add logging when RAM[0x3F] bit 7 is set (copy request):
```cpp
// In memory write_external when address == 0x3F
if ((value & 0x80) && !(old_value & 0x80)) {
    // Log: "COPY REQUESTED: count=RAM[0x7F], target=RAM[0x7E]"
}
```

### Step 3: Verify Copy Mode Control Signals
Check that Port 1 is set correctly during copy:
- P16 (bit 6) = 1: Copy mode enabled
- P13 (bit 3) = 0: VDC enabled
- P14 (bit 4) = 0: RAM enabled

### Step 4: Test with Breakpoint
Run emulator with breakpoint at BIOS 0x0A3 (copying_code) around frame 56:
```bash
videopac --debug --break 0x0a3 --frames 60 ...
```

### Step 5: Compare Frame 55 vs Frame 56
- Frame 55: Timer shows "02:00" (working)
- Frame 56: Timer should show "01:59" (corrupted)
- Compare VDC quad register contents between these frames

## Expected vs Actual

### Expected (Frame 56, timer = "01:59")
VDC Quad registers 0x42-0x49 should contain:
- 0x42-0x45: Digit '0' (minutes high)
- 0x46-0x49: Digit '1' (minutes low)
- 0x4A-0x4D: Digit '5' (seconds high)
- 0x4E-0x51: Digit '9' (seconds low)

### Actual (Frame 56, corrupted)
Need to dump VDC registers to see what's actually there.

## Files to Examine
- `src/memory.cpp::write_external()` - Copy mode handling
- `src/vdc.cpp::write_register()` - VDC register writes
- `src/cpu.cpp` - Port 1 handling
- BIOS 0x0A3-0x0AE - copying_code implementation
