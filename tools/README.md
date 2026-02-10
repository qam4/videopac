# Development Tools

This directory contains development tools for the Videopac/Odyssey 2 emulator.

## disasm_tool

A disassembler for Intel 8048 binaries (BIOS ROMs and game cartridges).

### Features

- Disassembles Intel 8048 machine code to assembly
- Automatically generates labels for jump/call targets
- Supports custom address ranges
- Works with both BIOS (starting at 0x000) and game ROMs (starting at 0x400)

### Building

The tool is built automatically when you build the project:

```bash
cmake -B build
cmake --build build
```

The compiled binary will be at `build/disasm_tool`.

### Usage

```bash
disasm_tool <rom_file> [start_addr] [end_addr]
```

**Arguments:**
- `rom_file`: Path to the binary file to disassemble
- `start_addr`: (Optional) Starting address in hex (default: 0x000)
- `end_addr`: (Optional) Ending address in hex (default: file size)

### Examples

**Disassemble BIOS (starts at 0x000):**
```bash
./build/disasm_tool "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" > french_bios_disasm.txt
```

**Disassemble game ROM (starts at 0x400):**
```bash
./build/disasm_tool "roms/Satellite Attack (1981)(Philips)(EU).bin" 0x400 > satellite_attack_disasm.txt
```

**Disassemble specific range:**
```bash
./build/disasm_tool "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" 0x0B0 0x100
```

### Output Format

The tool generates assembly with automatic labels:

```
cold_boot:
0x000: 84 00  JMP restart
0x002: 00     NOP

external_t0_interrupt:
0x003: 84 02  JMP vblank_external_interrupt
...
loc_0018:
0x012: 72 18  JB3 loc_0018
```

**Features:**
- Labels are placed on a separate line before the instruction
- Known BIOS/cartridge addresses use meaningful names (e.g., `cold_boot`, `restart`)
- Other jump targets use format: `loc_XXXX:` where XXXX is the hex address
- Jump/call operands use label names instead of hex addresses
- Only addresses that are actual jump/call targets get labels

### Notes

- The disassembler correctly handles JMP/CALL instructions using bits 5-7 of the opcode
- Conditional jumps (JB, JZ, etc.) use page-relative addressing
- Labels are only generated for addresses within the disassembled range

## annotate_disasm.py

A Python script to merge disassembly output with comments from o2romsrc.txt.

### Usage

```bash
python3 tools/annotate_disasm.py <disasm_file> <o2romsrc_file> > output.txt
```

### Complete Workflow for Annotated French BIOS

```bash
# Step 1: Generate disassembly with labels
./build/disasm_tool "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" > french_bios_disasm.txt

# Step 2: Add comments from US BIOS source
python3 tools/annotate_disasm.py french_bios_disasm.txt doc/o2romsrc.txt > french_bios_annotated.txt
```

This produces a fully annotated disassembly with:
- Correct JMP/CALL addresses
- Automatic labels for all jump targets
- Block comments (lines starting with `;`)
- Inline comments from the US BIOS source
