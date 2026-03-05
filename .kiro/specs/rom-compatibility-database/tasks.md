# Implementation Plan: ROM Compatibility Database

## Overview

Implement a CSV-based ROM compatibility database and a Python CLI tool (`tools/crc32_tool.py`) with scan, populate, and validate modes. The tool uses only Python stdlib (`zipfile`, `zlib`, `csv`, `argparse`). Property-based tests use Hypothesis, unit tests use pytest.

## Tasks

- [x] 1. Create data models and CRC32 computation core
  - [x] 1.1 Create `tools/crc32_tool.py` with data model classes (`RomInfo`, `CsvRow`, `ValidationReport`) and CRC32 computation functions (`compute_crc32`, `compute_crc32_from_bytes`)
    - Define `RomInfo` dataclass with fields: filename, crc32, size, source, zip_path
    - Define `CsvRow` dataclass with fields: crc32, name, filename, size, platform, region, status, alt_filenames, notes
    - Define `ValidationReport` dataclass with fields: crc_mismatches, missing_roms, orphan_roms, status_counts
    - Implement `compute_crc32(filepath)` using `zlib.crc32`, returning 8-char uppercase hex
    - Implement `compute_crc32_from_bytes(data)` returning 8-char uppercase hex
    - _Requirements: 1.1, 1.2, 3.1_

  - [ ]* 1.2 Write property test for CRC32 format invariant
    - **Property 2: CRC32 Format Invariant**
    - Test that for any byte sequence (0–16384 bytes), `compute_crc32_from_bytes` returns exactly 8 uppercase hex characters
    - **Validates: Requirements 3.1**

- [x] 2. Implement scan mode — directory and zip scanning
  - [x] 2.1 Implement `scan_directory(dirpath)` and `scan_zip(zippath)` functions
    - `scan_directory`: recursively walk directory, compute CRC32 for all `.bin` files (case-insensitive match)
    - `scan_zip`: open zip archive, extract `.bin` entries, compute CRC32 from extracted bytes
    - Handle errors gracefully: log warnings for unreadable files and corrupt zips, continue processing
    - _Requirements: 3.2, 3.3, 3.4, 3.6_

  - [x] 2.2 Implement `scan_all(roms_dir, bios_dir, tosec_dir)` function
    - Scan loose `.bin` files in roms_dir (source="local") and bios_dir (source="bios")
    - Scan all `.zip` files in tosec_dir (source="tosec"), tracking zip_path
    - Return combined list of `RomInfo` entries
    - _Requirements: 3.2, 3.4, 4.1_

  - [ ]* 2.3 Write property test for scan completeness
    - **Property 8: Scan Completeness**
    - Generate temp directory trees with `.bin` files and `.zip` archives containing `.bin` files
    - Verify scan returns a `RomInfo` for every `.bin` file present
    - **Validates: Requirements 3.2, 3.3, 3.4, 4.1**

  - [ ]* 2.4 Write unit tests for scan error handling
    - Test with corrupt zip files, empty directories, zip with no `.bin` files, non-`.bin` files skipped
    - _Requirements: 3.6_

- [x] 3. Implement Gamelist parser and platform/region detection
  - [x] 3.1 Implement `parse_gamelist(filepath)` function
    - Parse `Gamelist777.txt` format: lines with `.bin` followed by whitespace then description
    - Return dict mapping filename stem → game name
    - Read with `latin-1` encoding for ISO-8859-1 characters
    - Skip blank lines, section headers, and comment-style lines
    - _Requirements: 4.3_

  - [x] 3.2 Implement `detect_platform(filename)` and `detect_region(filename, source)` functions
    - Platform detection from filename prefix: `vp_`→VP, `o2_`→O2, `jo_`→Jopac, `br_`→Brazilian, `pb_`→VP, `im_`→VP, `bios_`→BIOS, `pr_`→VP, `mod_`→VP, `new_`→VP, `ntsc_`→O2, `pal_`→VP; `_pl` suffix → VP+
    - Region detection: TOSEC `(EU)`, `(US)`, `(FR)`, `(BR)`, `(CA)`, `(EU-US)` parsing; local prefix-based (`br_`→BR, `ntsc_`→US, `_F.bin`→FR, default EU for VP)
    - _Requirements: 1.1_

  - [ ]* 3.3 Write unit tests for Gamelist parsing, platform detection, and region detection
    - Test each filename prefix maps to correct platform
    - Test TOSEC region parsing and local filename region conventions
    - Test Gamelist format edge cases (multi-word names, parenthetical notes, encoding)
    - _Requirements: 1.1, 4.3_

- [x] 4. Checkpoint — Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 5. Implement populate mode — deduplication, merging, and CSV output
  - [x] 5.1 Implement `deduplicate_by_crc(roms)` function
    - Group `RomInfo` entries by CRC32
    - For each CRC32 group, merge filenames; prefer local source as primary
    - Log warnings for duplicate CRC32 within same source
    - _Requirements: 1.4, 4.2_

  - [ ]* 5.2 Write property test for deduplication uniqueness
    - **Property 3: Deduplication Uniqueness**
    - Verify every CRC32 appears exactly once in output and all unique CRC32s from input are represented
    - **Validates: Requirements 1.4, 4.2**

  - [x] 5.3 Implement `merge_with_gamelist(entries, gamelist)` function
    - Assign game names from Gamelist when local filename matches
    - Use TOSEC filename stem as name for TOSEC-only ROMs
    - Use filename as name for local ROMs with no Gamelist match
    - Record TOSEC filenames as semicolon-separated alt_filenames
    - Set status to `not_tested` for all entries
    - Apply `detect_platform` and `detect_region` to each entry
    - _Requirements: 2.2, 4.3, 4.4, 4.5, 4.6_

  - [ ]* 5.4 Write property test for cross-source merge
    - **Property 5: Cross-Source Merge Produces Single Entry with Correct Names**
    - Verify that when same CRC32 appears in local and TOSEC with a Gamelist match, the name comes from Gamelist and TOSEC filename is in alt_filenames
    - **Validates: Requirements 1.7, 4.4**

  - [ ]* 5.5 Write property test for name resolution
    - **Property 6: Name Resolution Always Produces a Non-Empty Name**
    - Verify all three name resolution paths produce non-empty names
    - **Validates: Requirements 4.3, 4.5, 4.6**

  - [ ]* 5.6 Write property test for default status
    - **Property 7: New Entries Default to Valid Status**
    - Verify every entry from populate has status `not_tested` and status is one of the 7 valid values
    - **Validates: Requirements 2.1, 2.2**

- [x] 6. Implement CSV I/O and sorting
  - [x] 6.1 Implement `write_csv(rows, output_path)` and `load_csv(filepath)` functions
    - Write CSV with header: `crc32,name,filename,size,platform,region,status,alt_filenames,notes`
    - Handle quoting for fields containing commas, quotes, and special characters
    - Sort entries by platform category (BIOS, O2, VP, VP+, Jopac, Brazilian) then alphabetically by name (case-insensitive)
    - `load_csv` parses CSV back into `CsvRow` list, logging warnings for malformed rows
    - _Requirements: 1.2, 1.3, 1.6_

  - [ ]* 6.2 Write property test for CSV round-trip preservation
    - **Property 1: CSV Round-Trip Preservation**
    - Generate random `CsvRow` lists (including commas, quotes, special chars in notes), write to CSV, read back, verify equivalence
    - **Validates: Requirements 1.1, 1.2, 2.3**

  - [ ]* 6.3 Write property test for sort invariant
    - **Property 4: Sort Invariant**
    - Verify entries within each platform group are sorted alphabetically by name (case-insensitive)
    - **Validates: Requirements 1.6**

- [x] 7. Checkpoint — Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 8. Implement validate mode
  - [x] 8.1 Implement `validate(database, scanned)` function
    - Compare database entries against scanned ROMs by CRC32
    - Report CRC mismatches (filename, expected CRC, actual CRC)
    - Report missing ROMs (DB entries with no corresponding file)
    - Report orphan ROMs (files on disk not in DB)
    - Compute status summary counts
    - _Requirements: 5.1, 5.2, 5.3_

  - [ ]* 8.2 Write property test for validation detects CRC mismatches
    - **Property 9: Validation Detects All CRC Mismatches**
    - Generate database and ROM collection with known mismatches, verify all are reported
    - **Validates: Requirements 5.1**

  - [ ]* 8.3 Write property test for validation detects missing ROMs
    - **Property 10: Validation Detects All Missing ROMs**
    - Generate database with entries that have no corresponding ROM file, verify all are reported
    - **Validates: Requirements 5.2**

  - [ ]* 8.4 Write property test for status summary accuracy
    - **Property 11: Status Summary Accuracy**
    - Generate database, verify status counts match actual entry counts per status
    - **Validates: Requirements 5.3**

- [x] 9. Wire up CLI with argparse — scan, populate, validate subcommands
  - [x] 9.1 Implement argparse CLI entry point with three subcommands
    - `scan` subcommand: accepts path argument, calls scan functions, prints `filename CRC32` lines to stdout
    - `populate` subcommand: accepts `--roms-dir`, `--bios-dir`, `--tosec-dir`, `--gamelist`, `--output` options with defaults per design; orchestrates scan → parse gamelist → deduplicate → merge → write CSV
    - `validate` subcommand: accepts `--database`, `--roms-dir`, `--bios-dir`, `--tosec-dir` options with defaults; orchestrates load CSV → scan → validate → print report and status summary
    - Implement exit codes: 0 (success), 1 (fatal error), 2 (completed with warnings)
    - _Requirements: 3.1, 3.2, 3.5, 4.1, 5.1, 5.2, 5.3_

  - [ ]* 9.2 Write unit tests for CLI argument parsing and end-to-end mode integration
    - Test each subcommand with default and custom arguments
    - Test error handling for missing required files
    - Test exit codes for success, fatal error, and warnings
    - _Requirements: 3.5, 3.6_

- [x] 10. Final checkpoint — Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Property tests use Hypothesis in `tests/test_crc32_tool_props.py`; unit tests in `tests/test_crc32_tool.py`
- All code uses Python stdlib only (`zipfile`, `zlib`, `csv`, `argparse`, `dataclasses`, `pathlib`)
- Checkpoints ensure incremental validation
