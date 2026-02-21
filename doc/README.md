# Videopac Emulator Documentation

This directory contains technical documentation, hardware references, debugging guides, and case studies for the Videopac/Odyssey 2 emulator.

## Documentation Structure

### Hardware Reference (`hardware/`)
Technical specifications and architecture documentation for the Videopac/Odyssey 2 hardware:

- **[memory-architecture.md](hardware/memory-architecture.md)** - Complete memory map and architecture
- **[bios.md](hardware/bios.md)** - BIOS routines, interrupt handlers, and RAM usage
- **[8048.txt](hardware/8048.txt)** - Intel 8048 CPU reference
- **[8245.md](hardware/8245.md)** - Intel 8245 VDC (Video Display Controller) reference
- **[port1_bits.md](hardware/port1_bits.md)** - Port 1 control signals

### Debugging (`debugging.md` and `case-studies/`)
Guides and real-world examples for debugging games and emulator issues:

- **[debugging.md](debugging.md)** - Main debugging guide (will be replaced by comprehensive handbook)
- **Case Studies:**
  - **[road-movement-bug.md](case-studies/road-movement-bug.md)** - PAL/NTSC timing bug in Course de Voitures (BIOS frame counter issue)
  - **[satellite-attack.md](case-studies/satellite-attack.md)** - Frame-by-frame analysis of Satellite Attack
  - **[select-game.md](case-studies/select-game.md)** - Analysis of the game selection routine

### Reference Materials (`reference/`)
Manuals, specifications, and technical references:

- **[mcs-48-assembly-language-manual.md](reference/mcs-48-assembly-language-manual.md)** - Intel MCS-48 assembly language
- **[mcs-48-user-manual.md](reference/mcs-48-user-manual.md)** - Intel MCS-48 user manual
- **[o2doc.md](reference/o2doc.md)** - Odyssey 2 programming documentation
- **[trace_format.md](reference/trace_format.md)** - Trace log format specification
- **[Reading-8048-Series-Code.md](reference/Reading-8048-Series-Code.md)** - Guide to reading 8048 assembly

### Other Documentation
- **[french_bios_annotated.txt](french_bios_annotated.txt)** - Annotated French BIOS disassembly
- **[o2romsrc.txt](o2romsrc.txt)** - Odyssey 2 ROM source code
- **[satellite-attack-disassembly.txt](satellite-attack-disassembly.txt)** - Complete disassembly
- **PDFs:** Original hardware manuals and programming guides

## Quick Links

- [Main README](../README.md) - Project overview
- [Building](../BUILDING.md) - Build instructions
- [Testing](../TESTING.md) - Testing guide
- [Hacking](../HACKING.md) - Developer guide

## Contributing

When adding new documentation:
1. Place hardware specs in `hardware/`
2. Place debugging case studies in `case-studies/`
3. Place reference materials in `reference/`
4. Update this README with links to new documents
