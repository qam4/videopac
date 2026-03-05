# Requirements Document

## Introduction

A ROM compatibility database for the videopac emulator that catalogs official Odyssey 2 / Philips Videopac ROMs with their names, CRC32 checksums, and emulator support status. The database enables tracking which ROMs work correctly in the emulator, which have issues, and which remain untested. A companion CRC32 tool automates checksum computation from ROM files (including extraction from TOSEC zip archives), enabling the database to be populated and verified from the user's existing ROM collection.

The ROM collection exists in two locations with different naming conventions:
- `roms/` — loose .bin files and BIOS files, named per the Gamelist777.txt convention (e.g., `vp_01.bin`)
- `roms/Magnavox_Odyssey_2_TOSEC_2012_04_23/` — TOSEC-format zip archives with descriptive filenames (e.g., `Race - Spin-out - Cryptogram (1978)(Magnavox)(US).zip`)

Many ROMs appear in both collections under different filenames. CRC32 is the canonical identifier — same binary content produces the same CRC regardless of filename. The tool uses CRC to deduplicate across collections and track alternate names.

## Glossary

- **Database**: A structured data file (CSV, JSON, or Markdown table) stored in the repository that contains ROM entries with metadata and compatibility status
- **ROM_Entry**: A single record in the Database representing one ROM, including its name, CRC32 checksum, and compatibility status
- **CRC32_Tool**: A command-line utility (Python script) that computes CRC32 checksums from .bin ROM files and from .bin files inside zip archives
- **CRC32_Checksum**: A 32-bit cyclic redundancy check value represented as an 8-character uppercase hexadecimal string (e.g., "A1B2C3D4"), used to uniquely identify a ROM's binary content
- **TOSEC_Set**: The TOSEC-format ROM collection located in `roms/Magnavox_Odyssey_2_TOSEC_2012_04_23/`, containing zip files with .bin ROMs inside, using descriptive TOSEC naming conventions
- **Local_ROM_Set**: The loose .bin ROM and BIOS files in `roms/` and `roms/BIOS/`, named per the Gamelist777.txt convention
- **Gamelist**: The existing game list file at `roms/Gamelist777.txt` containing ~220 game entries with names and descriptions but no CRC32 checksums; covers the Local_ROM_Set
- **Compatibility_Status**: One of the defined status categories indicating how well the Emulator runs a given ROM:
  - `perfect` — Runs identically to real hardware. No known issues.
  - `playable` — Fully playable from start to finish. Minor cosmetic issues may exist (e.g., slight color difference, minor sprite flicker timing).
  - `in_game` — Gets past title/menu screen and into gameplay, but has issues that affect playability (e.g., missing collision detection, broken scoring, input not working for certain keys).
  - `menu` — Boots and shows title/menu screen but cannot progress into gameplay.
  - `boots` — Shows something on screen (BIOS screen, garbage, partial graphics) but doesn't reach a usable menu.
  - `nothing` — Black screen, crash, or immediate hang. No visible output.
  - `not_tested` — No one has tried this ROM yet.
- **Emulator**: The videopac C++ Odyssey 2 / Philips Videopac emulator with SDL and libretro frontends
- **ROM_File**: A binary file (typically .bin, 2KB–16KB) containing the program data for an Odyssey 2 / Videopac cartridge

## Requirements

### Requirement 1: Database File Structure

**User Story:** As a developer, I want a structured data file in the repository that lists all known ROMs with their metadata, so that I can track emulator compatibility at a glance.

#### Acceptance Criteria

1. THE Database SHALL store each ROM_Entry with the following fields: game name (human-readable title), primary filename, CRC32_Checksum, ROM file size in bytes, Compatibility_Status, platform (O2/VP/VP+/Jopac/Brazilian), region, alternate filenames, and optional notes
2. THE Database SHALL use CSV format with the following columns: `crc32`, `name`, `filename`, `size`, `platform`, `region`, `status`, `alt_filenames`, `notes`
3. THE Database SHALL be stored at `doc/compatibility.csv` in the repository
3. THE Database SHALL include entries for all unique ROMs found across both the Local_ROM_Set and TOSEC_Set, deduplicated by CRC32_Checksum
4. WHEN a ROM_Entry has no CRC32_Checksum computed, THE Database SHALL store an empty checksum field rather than omitting the entry
5. THE Database SHALL sort entries alphabetically by game name within each platform category
6. WHEN the same CRC32_Checksum appears under different filenames across the Local_ROM_Set and TOSEC_Set, THE Database SHALL store a single ROM_Entry with the primary name from the Gamelist and the alternate filenames in a separate field

### Requirement 2: Compatibility Status Tracking

**User Story:** As a developer, I want to record the emulator's support status for each ROM, so that I can identify which games need work and track progress over time.

#### Acceptance Criteria

1. THE Database SHALL support the following Compatibility_Status values: `perfect`, `playable`, `in_game`, `menu`, `boots`, `nothing`, `not_tested`
2. WHEN a new ROM_Entry is added to the Database, THE Database SHALL assign `not_tested` as the default Compatibility_Status
3. THE Database SHALL allow a free-text notes field per ROM_Entry to describe specific issues (e.g., "Voice module sounds missing", "Crashes after level 3", "Plus graphics not rendered")
4. WHEN a ROM_Entry has Compatibility_Status other than `perfect` or `not_tested`, THE Database SHOULD contain a notes field describing the observed behavior

### Requirement 3: CRC32 Computation Tool

**User Story:** As a developer, I want a tool that computes CRC32 checksums from my ROM files, so that I can populate and verify the database automatically.

#### Acceptance Criteria

1. WHEN given a path to a .bin ROM_File, THE CRC32_Tool SHALL compute and output the CRC32_Checksum as an 8-character uppercase hexadecimal string
2. WHEN given a path to a directory, THE CRC32_Tool SHALL compute CRC32 checksums for all .bin files in that directory recursively
3. WHEN given a path to a zip file containing .bin files, THE CRC32_Tool SHALL extract and compute CRC32 checksums for each .bin file inside the archive
4. WHEN given a path to a directory containing zip files, THE CRC32_Tool SHALL process all zip files and compute CRC32 checksums for the .bin files inside each archive
5. THE CRC32_Tool SHALL output results in a format that can be used to update the Database (one line per ROM with filename and CRC32_Checksum)
6. IF a file cannot be read or a zip archive is corrupt, THEN THE CRC32_Tool SHALL print an error message identifying the problematic file and continue processing remaining files

### Requirement 4: Database Population from ROM Collection

**User Story:** As a developer, I want to populate the database from my existing ROM collection and game list, so that I have a complete starting point without manual data entry.

#### Acceptance Criteria

1. WHEN the CRC32_Tool is run in populate mode, THE CRC32_Tool SHALL scan both the Local_ROM_Set and the TOSEC_Set to compute CRC32 checksums for all ROM files
2. THE CRC32_Tool SHALL deduplicate ROMs across both collections by CRC32_Checksum — ROMs with identical CRC32 values SHALL produce a single Database entry
3. THE CRC32_Tool SHALL merge CRC32 results with game names and descriptions from the Gamelist, matching by filename stem (e.g., `vp_01.bin` matches the Gamelist entry for `vp_01.bin`)
4. WHEN a ROM appears in both collections with different filenames but the same CRC32, THE CRC32_Tool SHALL use the Gamelist name as primary and record the TOSEC filename as an alternate
5. WHEN a ROM file exists in the TOSEC_Set but has no matching CRC32 in the Local_ROM_Set, THE CRC32_Tool SHALL include the ROM in the Database using the TOSEC filename as the game name
6. WHEN a ROM file exists in the Local_ROM_Set but has no matching Gamelist entry, THE CRC32_Tool SHALL include the ROM in the Database using the filename as the game name

### Requirement 5: Database Validation

**User Story:** As a developer, I want to verify that the database is consistent and complete, so that I can trust the data when making emulator development decisions.

#### Acceptance Criteria

1. WHEN the CRC32_Tool is run in validation mode against a ROM directory, THE CRC32_Tool SHALL report any ROM files whose computed CRC32 does not match the Database entry
2. WHEN the CRC32_Tool is run in validation mode, THE CRC32_Tool SHALL report any Database entries that have no corresponding ROM file in the scanned directories
3. THE CRC32_Tool SHALL report a summary count of ROMs by Compatibility_Status (e.g., "120 not_tested, 50 perfect, 30 playable, 10 in_game, 5 menu, 3 boots, 2 nothing")
