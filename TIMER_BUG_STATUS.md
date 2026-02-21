# Timer Bug Investigation Status

## Current Status
Investigation in progress. Identified the timer display mechanism but unable to confirm VDC writes due to logging issues.

## Key Findings

### 1. Timer Display Mechanism
The timer in Course de Voitures uses the BIOS RAM-to-VDC copy mechanism:
- BIOS routine `display_2_digit_bcd_characters` (0x17C) prepares data in external RAM
- Data is stored backwards in RAM[0x7D-0x7A]
- RAM[0x7E] contains target VDC address (should be 0x42 for timer quads)
- RAM[0x7F] contains byte count (8 bytes for 4 digits)
- RAM[0x3F] bit 7 is set to request copy during next VBLANK
- VBLANK interrupt handler calls `copying_code` (0x0A3)
- Copy loop uses MOVX @R1,A with P1=0xE7 (copy mode: P16=1, P13=0, P14=0)

### 2. Bug Manifestation
- Timer starts at "02:00"
- Corruption begins at frame 56 (~1 second into gameplay)
- Frame 56 is when timer first counts down from "02:00" to "01:59"
- This is the FIRST time all 4 digits change simultaneously
- Timer becomes corrupted showing random characters

### 3. Trace Analysis
From `trace.log` (frames 54-69):
- MOVX @R1,A instructions (opcode 0x91) at BIOS address 0x0A9
- Port 1 = 0xE7 during copy (binary 11100111):
  - P16 (bit 6) = 1: Copy mode enabled
  - P13 (bit 3) = 0: VDC enabled  
  - P14 (bit 4) = 0: RAM enabled
- Copy mechanism IS being triggered

### 4. Hypothesis
The bug is in the emulator's RAM-to-VDC copy mode implementation in `src/memory.cpp::write_external()`.

Possible causes:
1. Copy mode check logic incorrect
2. VDC address auto-increment not working
3. Data corruption during multi-byte copy
4. Timing issue with VBLANK copy

## Attempted Debugging

### Logging Attempts (All Failed)
1. Added logging to `src/memory.cpp::write_external()` - no output
2. Added logging to `src/vdc.cpp::write_register()` - no output  
3. Added file logging with fopen() - files not created
4. Added file logging with std::ofstream - files not created
5. Added logging to VDC constructor - file not created

**Issue**: File I/O logging is not working for unknown reasons. stdout/stderr logging also not appearing.

## Recommended Next Steps

### Option 1: Use Existing Debugger
The emulator has a built-in debugger with VDC dump command:
```bash
videopac --debug --break <address> --bios <bios> <rom>
# Then use "vdc" command to dump VDC registers
```

Run with breakpoint at frame 56 to examine VDC quad registers (0x42-0x51).

### Option 2: Add VDC Register Dump to Screenshots
Modify the screenshot code to dump VDC registers to a text file alongside each screenshot:
```cpp
// In save_screenshot():
std::ofstream vdc_dump(filename + ".vdc.txt");
for (int i = 0x40; i <= 0x51; i++) {
    vdc_dump << "VDC[0x" << std::hex << i << "] = 0x" 
             << (int)vdc.read_register(i) << std::endl;
}
```

### Option 3: Examine Copy Mode Logic
Review `src/memory.cpp::write_external()` copy mode implementation:
- Verify `copy_mode()`, `vdc_enabled()`, `ram_enabled()` return correct values
- Check if VDC write_register() is actually being called
- Add assertions or breakpoints in copy mode path

### Option 4: Compare with Working Emulator
- Test same ROM in o2em reference emulator
- Compare VDC register values at frame 56
- Identify differences in copy mode handling

## Files Modified (Need Cleanup)
- `src/memory.cpp` - Added debug logging (remove before commit)
- `src/vdc.cpp` - Added debug logging (remove before commit)

## Investigation Documents Created
- `TIMER_BUG_INVESTIGATION.md`
- `TIMER_BUG_ANALYSIS.md`
- `TIMER_BUG_FINDINGS.md`
- `TIMER_BUG_ROOT_CAUSE.md`
- `TIMER_BUG_HYPOTHESIS.md`
- `TIMER_BUG_NEXT_STEPS.md`
- `TIMER_BUG_STATUS.md` (this file)
- `dump_vdc_state.py` - Extracts timer region from screenshots
- `timer_frame_*.png` - Timer regions from frames 54-58

## Key Code Locations
- `src/memory.cpp::write_external()` - Copy mode handling (lines 153-180)
- `src/vdc.cpp::write_register()` - VDC register writes (line 166)
- `doc/french_bios_annotated.txt` - BIOS documentation
  - 0x0A3: copying_code (RAM-to-VDC copy loop)
  - 0x17C: display_2_digit_bcd_characters
  - 0x1B0: up_down_counter
- `doc/hardware/bios.md` - RAM-to-VDC copy mechanism documentation
