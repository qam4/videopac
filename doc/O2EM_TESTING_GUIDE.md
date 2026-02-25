# O2EM Testing Guide for Killer Bees Investigation

## Goal
Verify if Killer Bees actually works in o2em and capture key information about how it runs.

## Compile Flags for Debugging

### O2EM_COMPAT
When comparing traces with o2em, you can enable o2em compatibility mode:

```bash
# Method 1: Add to CMakeUserPresets.json (recommended)
# Edit CMakeUserPresets.json and add to your preset's cacheVariables:
"O2EM_COMPAT": "ON"

# Then reconfigure and rebuild:
cmake --preset=dev-mingw
cmake --build --preset=dev-mingw

# Method 2: Command line (temporary, not persistent)
cmake -B build -DO2EM_COMPAT=ON
cmake --build build
```

This enables o2em-specific behaviors:
- VBlank timing: Changes from scanline 240 (hardware spec) to scanline 242 (o2em implementation)
- Timer quirk: STOP TCNT only stops timer mode, leaving counter mode active (deviates from Intel 8048 spec)

Use this when doing trace comparison debugging to minimize divergences.

**Default behavior:** Hardware-accurate (Intel 8048 spec + scanline 240)
**O2EM compatibility:** O2EM quirks (timer behavior + scanline 242)

## Setup (Linux)

### 1. Install Dependencies
```bash
# Amazon Linux (using yum)
sudo yum install gcc make allegro-devel

# If allegro-devel not found, enable EPEL first:
# sudo yum install epel-release
# sudo yum install allegro-devel

# Ubuntu/Debian
sudo apt-get install build-essential liballegro4-dev

# Fedora/RHEL
sudo dnf install allegro-devel gcc make

# Arch
sudo pacman -S allegro4 base-devel
```

### 2. Build O2EM
```bash
cd doc/o2em
make
```

### 3. Copy ROM and BIOS
```bash
# Copy Killer Bees ROM
cp "../../roms/Killer Bees (1983)(Philips)(US).bin" ./

# Copy BIOS (o2em needs this)
cp ../../roms/BIOS/bios_O2rom.bin ./o2rom.bin
```

## Test 1: Does Killer Bees Work?

### Run the game normally:
```bash
./o2em "Killer Bees (1983)(Philips)(US).bin"
```

**What to check:**
- Does the game start?
- Do you see graphics (blue background, white band, "KILLER BEES" text)?
- Does the game respond to input?
- Take a screenshot if possible

**Expected behavior:**
- Should show title screen with "KILLER BEES" text
- Should have colored graphics (not just black screen)

## Test 2: Use the Debugger

### Start with debugger:
```bash
./o2em -debug "Killer Bees (1983)(Philips)(US).bin"
```

### In the debugger, run these commands:

#### A. Check initial state
```
reg
```
**Look for:** PC, A, P1, P2 values at start

#### B. Set breakpoint at ROM entry and run
```
bp 400
q
```
**This runs to address 0x400 (ROM entry point)**

#### C. Check registers after ROM entry
```
reg
```
**Record:** PC, P1 value

#### D. Step through first 20 instructions
```
go 20
reg
```
**Record:** Final PC, P1 value

#### E. Check current bank
The debugger shows registers. Look at P1 value:
- Calculate bank: `(~P1) & 0x03`
- Example: If P1=0xAC (binary 10101100), then:
  - ~P1 = 0x53 (binary 01010011)
  - (~P1) & 0x03 = 0x03 & 0x03 = 3
  - So bank = 3

#### F. Run to first VDC write
```
bp A0
q
```
**This breaks when VDC register 0xA0 is written**

```
reg
```
**Record:** PC where VDC write happens, P1 value

#### G. Check if game switches banks
```
bpc
go 1000
reg
```
**Record:** P1 value after 1000 instructions
**Check:** Did P1 bits 0-1 change? (indicates bank switch)

## Test 3: Add Trace Logging (Optional)

If you want to add trace logging to o2em:

### Edit cpu.c
Find the `cpu_exec()` function and add at the start:
```c
// Add at top of file
static FILE *trace_fp = NULL;

// Add at start of cpu_exec()
if (!trace_fp) {
    trace_fp = fopen("o2em_trace.log", "w");
}
if (trace_fp && master_clk < 100000) {  // Only first 100k cycles
    int bank = 0;
    if (app_data.bank == 3) {
        bank = (~p1) & 0x03;
    }
    fprintf(trace_fp, "%03X %02X %02X %d\n", pc, p1, acc, bank);
}
```

### Rebuild and run:
```bash
make clean
make
./o2em "Killer Bees (1983)(Philips)(US).bin"
```

This creates `o2em_trace.log` with format: `PC P1 A BANK`

## What We're Looking For

### Critical Questions:
1. **Does Killer Bees work in o2em?**
   - If YES: We have a bug in our emulator
   - If NO: The ROM might be bad or o2em has same issue

2. **What is P1 value during execution?**
   - Especially bits 0-1 (P10, P11) which control banking
   - Does it change during execution?

3. **What bank is active?**
   - Calculate: `(~P1) & 0x03`
   - Does it stay at bank 0 or switch to other banks?

4. **When does first VDC write happen?**
   - At what PC address?
   - What is P1 value at that time?

5. **Does the game get stuck?**
   - If you let it run, does it freeze?
   - At what PC address?

## Expected Results

Based on our analysis, we expect:
- Game should work in o2em (otherwise ROM is bad)
- P1 should change during execution (bank switching)
- Bank should alternate between 0 and other values
- VDC writes should happen early (within first few thousand instructions)

## Report Back

Please provide:
1. Does the game work? (screenshot if possible)
2. Initial P1 value at ROM entry (0x400)
3. P1 value after 1000 instructions
4. PC address of first VDC write
5. Any error messages or crashes

This will tell us if:
- The ROM is good
- Banking is working in o2em
- What we're doing differently
