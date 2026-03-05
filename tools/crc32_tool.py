#!/usr/bin/env python3
"""ROM Compatibility Database Tool.

Computes CRC32 checksums from ROM files, populates and validates
the compatibility database (doc/compatibility.csv).
"""

from __future__ import annotations

import warnings
import zipfile
import zlib
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional


# ---------------------------------------------------------------------------
# Data models
# ---------------------------------------------------------------------------

@dataclass
class RomInfo:
    """Intermediate scan result for a single ROM file."""
    filename: str           # e.g. "vp_01.bin" or TOSEC name
    crc32: str              # 8-char uppercase hex
    size: int               # file size in bytes
    source: str             # "local", "bios", or "tosec"
    zip_path: Optional[str] = None  # path to containing zip, if from TOSEC


@dataclass
class CsvRow:
    """A single entry in the compatibility database."""
    crc32: str              # 8-char uppercase hex, or empty string
    name: str               # human-readable game title
    filename: str           # primary filename
    size: int               # ROM file size in bytes
    platform: str           # O2, VP, VP+, Jopac, Brazilian, BIOS
    region: str             # US, EU, FR, BR, CA, EU-US, etc.
    status: str             # one of the 7 Compatibility_Status values
    alt_filenames: str = ""  # semicolon-separated alternate filenames
    notes: str = ""  # free-text notes


@dataclass
class MergedEntry:
    """Result of deduplicating RomInfo entries by CRC32."""
    crc32: str              # 8-char uppercase hex
    primary_filename: str   # preferred filename (local > bios > tosec)
    all_filenames: list[str] = field(default_factory=list)  # every filename seen
    primary_source: str = ""  # source of the primary filename
    size: int = 0           # ROM file size in bytes


@dataclass
class ValidationReport:
    """Result of validating the database against scanned ROM files."""
    # (filename, expected_crc, actual_crc)
    crc_mismatches: list[tuple[str, str, str]] = field(default_factory=list)
    # DB entries with no ROM file found
    missing_roms: list[str] = field(default_factory=list)
    # (filename, crc) on disk but not in DB
    orphan_roms: list[tuple[str, str]] = field(default_factory=list)
    # status -> count
    status_counts: dict[str, int] = field(default_factory=dict)


# ---------------------------------------------------------------------------
# CRC32 computation
# ---------------------------------------------------------------------------

def compute_crc32(filepath: str) -> str:
    """Compute CRC32 of a file, return 8-char uppercase hex string."""
    data = Path(filepath).read_bytes()
    return compute_crc32_from_bytes(data)


def compute_crc32_from_bytes(data: bytes) -> str:
    """Compute CRC32 of raw bytes, return 8-char uppercase hex string."""
    crc = zlib.crc32(data) & 0xFFFFFFFF
    return f"{crc:08X}"


# ---------------------------------------------------------------------------
# Directory and zip scanning
# ---------------------------------------------------------------------------

def _is_bin_file(name: str) -> bool:
    """Check if a filename ends with .bin (case-insensitive)."""
    return name.lower().endswith(".bin")


def scan_directory(dirpath: str) -> list[RomInfo]:
    """Recursively scan directory for .bin files, compute CRC32 for each.

    Logs warnings for unreadable files and continues processing.
    """
    results: list[RomInfo] = []
    root = Path(dirpath)
    if not root.is_dir():
        return results
    for filepath in sorted(root.rglob("*")):
        if not filepath.is_file() or not _is_bin_file(filepath.name):
            continue
        try:
            data = filepath.read_bytes()
            crc = compute_crc32_from_bytes(data)
            results.append(RomInfo(
                filename=filepath.name,
                crc32=crc,
                size=len(data),
                source="local",
            ))
        except OSError as exc:
            warnings.warn(f"Cannot read file {filepath}: {exc}")
    return results


def scan_zip(zippath: str) -> list[RomInfo]:
    """Extract .bin files from zip archive, compute CRC32 for each.

    Logs warnings for corrupt zips and continues processing.
    """
    results: list[RomInfo] = []
    try:
        with zipfile.ZipFile(zippath, "r") as zf:
            for info in zf.infolist():
                if info.is_dir() or not _is_bin_file(info.filename):
                    continue
                try:
                    data = zf.read(info.filename)
                    crc = compute_crc32_from_bytes(data)
                    # Use just the basename of the entry inside the zip
                    entry_name = Path(info.filename).name
                    results.append(RomInfo(
                        filename=entry_name,
                        crc32=crc,
                        size=len(data),
                        source="tosec",
                        zip_path=str(zippath),
                    ))
                except Exception as exc:
                    warnings.warn(
                        f"Cannot read entry {info.filename} "
                        f"in {zippath}: {exc}"
                    )
    except (zipfile.BadZipFile, OSError) as exc:
        warnings.warn(f"Cannot open zip {zippath}: {exc}")
    return results


# ---------------------------------------------------------------------------
# Combined scanning
# ---------------------------------------------------------------------------

def scan_all(roms_dir: str, bios_dir: str, tosec_dir: str) -> list[RomInfo]:
    """Scan all ROM sources, return combined list of RomInfo entries.

    - roms_dir: loose .bin files scanned with source="local"
    - bios_dir: loose .bin files scanned with source="bios"
    - tosec_dir: .zip files scanned with source="tosec", tracking zip_path
    """
    results: list[RomInfo] = []

    # Scan loose .bin files in roms_dir (source="local")
    for rom in scan_directory(roms_dir):
        rom.source = "local"
        results.append(rom)

    # Scan loose .bin files in bios_dir (source="bios")
    for rom in scan_directory(bios_dir):
        rom.source = "bios"
        results.append(rom)

    # Scan all .zip files in tosec_dir (source="tosec")
    tosec_root = Path(tosec_dir)
    if tosec_root.is_dir():
        for zpath in sorted(tosec_root.rglob("*.zip")):
            if not zpath.is_file():
                continue
            for rom in scan_zip(str(zpath)):
                rom.source = "tosec"
                results.append(rom)

    return results


# ---------------------------------------------------------------------------
# Gamelist parser
# ---------------------------------------------------------------------------

def parse_gamelist(filepath: str) -> dict[str, str]:
    """Parse Gamelist777.txt, return dict mapping filename stem -> game name.

    Handles the format::

        vp_01.bin        Race/Spin-out/Cryptogram
        vp_01hack.bin    Race/Spin-out/Cryptogram  (color hack)

    Lines with ``.bin`` followed by whitespace and description
    text are entries. Lines without ``.bin``, blank lines,
    section headers, and notes are skipped.
    The file is read with ``latin-1`` encoding.

    Returns:
        dict mapping filename stem (e.g. ``"vp_01"``) to game name/description.
    """
    import re

    gamelist: dict[str, str] = {}
    with open(filepath, encoding="latin-1") as fh:
        for line in fh:
            line = line.rstrip("\n\r")
            # Skip blank lines
            if not line.strip():
                continue
            # Must contain .bin to be a valid entry
            if ".bin" not in line.lower():
                continue
            # Match: filename.bin followed by whitespace then description
            m = re.match(
                r"^(\S+\.bin)\s{2,}(.*\S)", line, re.IGNORECASE,
            )
            if not m:
                # No description or is a note/list entry — skip
                continue
            filename = m.group(1)
            description = m.group(2).strip()
            # Extract stem (remove .bin extension)
            stem = re.sub(r"\.bin$", "", filename, flags=re.IGNORECASE)
            gamelist[stem] = description
    return gamelist


# ---------------------------------------------------------------------------
# Platform and region detection
# ---------------------------------------------------------------------------

def detect_platform(filename: str) -> str:
    """Detect platform from a ROM filename.

    Uses the filename prefix convention from the Gamelist:
    - ``vp_`` → VP, with ``_pl`` suffix → VP+
    - ``o2_`` → O2
    - ``jo_`` → Jopac (with ``_pl`` suffix → VP+)
    - ``br_`` → Brazilian
    - ``pb_`` → VP (Parker Brothers)
    - ``im_`` → VP (Imagic)
    - ``bios_`` → BIOS
    - ``pr_`` → VP (prototype)
    - ``mod_`` → VP (modified)
    - ``new_`` → VP (homebrew)
    - ``ntsc_`` → O2 (NTSC conversions)
    - ``pal_`` → VP (PAL conversions)

    Returns one of: ``"VP"``, ``"VP+"``, ``"O2"``, ``"Jopac"``,
    ``"Brazilian"``, ``"BIOS"``, or ``""`` (unknown).
    """
    stem = filename.rsplit(".", 1)[0] if "." in filename else filename
    lower = stem.lower()

    # Check for Plus version: any segment (split by _) ends with "pl"
    # e.g. vp_01pl, jo_basket-bowling_pl, mod_35pl_fix
    # Exclude false positives like "vp_55_12" (12k version)
    segments = lower.split("_")
    has_plus = any(seg.endswith("pl") for seg in segments)

    # Prefix-based detection (order matters: longer prefixes first)
    prefix_map = [
        ("bios_", "BIOS"),
        ("ntsc_", "O2"),
        ("mod_", "VP"),
        ("new_", "VP"),
        ("pal_", "VP"),
        ("vp_", "VP"),
        ("o2_", "O2"),
        ("jo_", "Jopac"),
        ("br_", "Brazilian"),
        ("pb_", "VP"),
        ("im_", "VP"),
        ("pr_", "VP"),
    ]

    for prefix, platform in prefix_map:
        if lower.startswith(prefix):
            # VP+ override for VP and Jopac prefixes
            if has_plus and platform in ("VP", "Jopac"):
                return "VP+"
            return platform

    return ""


def detect_region(filename: str, source: str) -> str:
    """Detect region from a ROM filename and its source.

    For TOSEC-sourced ROMs, parses region tags in parentheses:
    ``(EU)``, ``(US)``, ``(FR)``, ``(BR)``, ``(CA)``, ``(EU-US)``, etc.

    For local ROMs, uses prefix/suffix conventions:
    - ``br_`` prefix → BR
    - ``ntsc_`` prefix → US
    - ``_F.bin`` suffix → FR
    - Default for VP-family → EU

    Args:
        filename: The ROM filename (e.g. ``"vp_01.bin"`` or TOSEC name).
        source: One of ``"local"``, ``"bios"``, ``"tosec"``.

    Returns:
        Region string (e.g. ``"EU"``, ``"US"``, ``"FR"``, ``"BR"``),
        or ``""`` if unknown.
    """
    import re

    if source == "tosec":
        # Parse TOSEC region tags like (EU), (US), (EU-US), (FR), (BR), (CA)
        m = re.search(r"\(([A-Z]{2}(?:-[A-Z]{2})?)\)", filename)
        if m:
            return m.group(1)
        return ""

    # Local / BIOS files — use prefix/suffix conventions
    lower = filename.lower()
    if lower.startswith("br_"):
        return "BR"
    if lower.startswith("ntsc_"):
        return "US"
    if lower.endswith("_f.bin"):
        return "FR"
    # Default for most local ROMs is EU (Videopac is European)
    if lower.startswith(("vp_", "jo_", "pb_", "im_", "mod_", "new_",
                         "pal_", "pr_", "bios_")):
        return "EU"
    if lower.startswith("o2_"):
        return "US"
    return ""


# ---------------------------------------------------------------------------
# Deduplication
# ---------------------------------------------------------------------------

# Source priority: local > bios > tosec
_SOURCE_PRIORITY = {"local": 0, "bios": 1, "tosec": 2}


def deduplicate_by_crc(roms: list[RomInfo]) -> dict[str, MergedEntry]:
    """Group ROMs by CRC32, merge filenames, pick primary name.

    For each CRC32 group:
    - Merge all filenames into a single list.
    - Prefer local source as primary (local > bios > tosec).
    - Log warnings for duplicate CRC32 within the same source.

    Returns:
        dict mapping CRC32 -> MergedEntry.
    """
    from collections import defaultdict

    # Group by CRC32
    groups: dict[str, list[RomInfo]] = defaultdict(list)
    for rom in roms:
        groups[rom.crc32].append(rom)

    result: dict[str, MergedEntry] = {}
    for crc, group in groups.items():
        # Detect duplicates within same source
        seen_source: dict[str, str] = {}  # source -> first filename
        for rom in group:
            if rom.source in seen_source:
                warnings.warn(
                    f"Duplicate CRC32 {crc} in source '{rom.source}': "
                    f"'{rom.filename}' and '{seen_source[rom.source]}'"
                )
            else:
                seen_source[rom.source] = rom.filename

        # Pick primary: lowest source priority
        group_sorted = sorted(
            group, key=lambda r: _SOURCE_PRIORITY.get(r.source, 99)
        )
        primary = group_sorted[0]

        # Collect all unique filenames (preserving order, primary first)
        all_filenames: list[str] = [primary.filename]
        for rom in group:
            if rom.filename not in all_filenames:
                all_filenames.append(rom.filename)

        result[crc] = MergedEntry(
            crc32=crc,
            primary_filename=primary.filename,
            all_filenames=all_filenames,
            primary_source=primary.source,
            size=primary.size,
        )

    return result


# ---------------------------------------------------------------------------
# Merge with Gamelist
# ---------------------------------------------------------------------------

def merge_with_gamelist(
    entries: dict[str, MergedEntry],
    gamelist: dict[str, str],
) -> list[CsvRow]:
    """Assign game names from Gamelist, TOSEC names as alternates.

    Name resolution:
    - If the primary filename (stem) matches a Gamelist entry, use the
      Gamelist name.
    - If the ROM is TOSEC-only (no local/bios match), use the TOSEC
      filename stem (without .bin) as the name.
    - If the ROM is local/bios but has no Gamelist match, use the
      filename as the name.

    TOSEC filenames are recorded as semicolon-separated alt_filenames.
    All entries get status ``not_tested``.
    Platform and region are detected from the primary filename.

    Args:
        entries: dict of CRC32 -> MergedEntry from deduplicate_by_crc.
        gamelist: dict of filename stem -> game name from parse_gamelist.

    Returns:
        list of CsvRow entries.
    """
    import re

    rows: list[CsvRow] = []
    for crc, merged in entries.items():
        primary = merged.primary_filename
        stem = re.sub(r"\.bin$", "", primary, flags=re.IGNORECASE)

        # --- Name resolution ---
        if stem in gamelist:
            # Local filename matches Gamelist
            name = gamelist[stem]
        elif merged.primary_source == "tosec":
            # TOSEC-only ROM: use filename stem as name
            name = stem
        else:
            # Local/bios ROM with no Gamelist match: use filename
            name = primary

        # --- Alt filenames: all non-primary filenames, semicolon-separated ---
        alt = [f for f in merged.all_filenames if f != primary]
        alt_filenames = ";".join(alt)

        # --- Platform and region detection ---
        # For TOSEC-only ROMs, detect from the TOSEC filename
        platform = detect_platform(primary)
        region = detect_region(primary, merged.primary_source)

        rows.append(CsvRow(
            crc32=crc,
            name=name,
            filename=primary,
            size=merged.size,
            platform=platform,
            region=region,
            status="not_tested",
            alt_filenames=alt_filenames,
            notes="",
        ))

    return rows


# ---------------------------------------------------------------------------
# CSV I/O
# ---------------------------------------------------------------------------

# Platform sort order for CSV output
_PLATFORM_ORDER = {
    "BIOS": 0,
    "O2": 1,
    "VP": 2,
    "VP+": 3,
    "Jopac": 4,
    "Brazilian": 5,
}

CSV_HEADER = ["crc32", "name", "filename", "size", "platform", "region",
              "status", "alt_filenames", "notes"]


def write_csv(rows: list[CsvRow], output_path: str) -> None:
    """Write sorted CSV database to disk.

    Entries are sorted by:
    1. Platform category (BIOS, O2, VP, VP+, Jopac, Brazilian)
    2. Alphabetically by name within each platform (case-insensitive)

    Uses Python's csv module for proper quoting of fields containing
    commas, quotes, and special characters.
    """
    import csv

    # Sort: platform order first, then name (case-insensitive)
    sorted_rows = sorted(
        rows,
        key=lambda r: (
            _PLATFORM_ORDER.get(r.platform, 99),
            r.name.lower(),
        ),
    )

    out = Path(output_path)
    out.parent.mkdir(parents=True, exist_ok=True)

    with open(out, "w", newline="", encoding="utf-8") as fh:
        writer = csv.writer(fh)
        writer.writerow(CSV_HEADER)
        for row in sorted_rows:
            writer.writerow([
                row.crc32,
                row.name,
                row.filename,
                row.size,
                row.platform,
                row.region,
                row.status,
                row.alt_filenames,
                row.notes,
            ])


def load_csv(filepath: str) -> list[CsvRow]:
    """Load and parse compatibility.csv into a list of CsvRow entries.

    Logs warnings for malformed rows (wrong number of fields) and skips them.
    """
    import csv

    rows: list[CsvRow] = []
    path = Path(filepath)
    if not path.is_file():
        warnings.warn(f"CSV file not found: {filepath}")
        return rows

    with open(path, newline="", encoding="utf-8") as fh:
        reader = csv.reader(fh)
        header = next(reader, None)
        if header is None:
            warnings.warn(f"Empty CSV file: {filepath}")
            return rows

        for line_num, fields in enumerate(reader, start=2):
            if len(fields) != len(CSV_HEADER):
                warnings.warn(
                    f"Malformed row at line {line_num} in {filepath}: "
                    f"expected {len(CSV_HEADER)} fields, got {len(fields)}"
                )
                continue
            try:
                rows.append(CsvRow(
                    crc32=fields[0],
                    name=fields[1],
                    filename=fields[2],
                    size=int(fields[3]) if fields[3] else 0,
                    platform=fields[4],
                    region=fields[5],
                    status=fields[6],
                    alt_filenames=fields[7],
                    notes=fields[8],
                ))
            except (ValueError, IndexError) as exc:
                warnings.warn(
                    f"Error parsing row at line {line_num} in {filepath}: {exc}"
                )

    return rows


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------


def validate(
    database: list[CsvRow],
    scanned: list[RomInfo],
) -> ValidationReport:
    """Compare database against scanned ROMs, return report.

    Checks:
    - CRC mismatches: DB entry filename matches a scanned ROM filename
      but the CRC32 values differ.
    - Missing ROMs: DB entries whose filename (or alt filenames) do not
      correspond to any scanned ROM file.
    - Orphan ROMs: scanned files whose CRC32 is not present in the DB.
    - Status summary: count of entries per Compatibility_Status.

    Args:
        database: list of CsvRow entries from load_csv.
        scanned: list of RomInfo entries from scan_all.

    Returns:
        ValidationReport with mismatches, missing, orphans, counts.
    """
    report = ValidationReport()

    # Build lookup: filename -> RomInfo from scanned ROMs
    scanned_by_name: dict[str, RomInfo] = {}
    for rom in scanned:
        scanned_by_name[rom.filename] = rom

    # Build set of all DB CRC32 values
    db_crcs: set[str] = set()

    for entry in database:
        if entry.crc32:
            db_crcs.add(entry.crc32)

        # Collect all filenames for this entry (primary + alts)
        filenames = [entry.filename]
        if entry.alt_filenames:
            filenames.extend(entry.alt_filenames.split(";"))

        # Check if any filename matches a scanned ROM
        found = False
        for fn in filenames:
            fn = fn.strip()
            if not fn:
                continue
            if fn in scanned_by_name:
                found = True
                rom = scanned_by_name[fn]
                # Check CRC mismatch
                if entry.crc32 and rom.crc32 != entry.crc32:
                    report.crc_mismatches.append(
                        (fn, entry.crc32, rom.crc32)
                    )
                break

        if not found:
            report.missing_roms.append(entry.filename)

    # Orphan ROMs: scanned files whose CRC32 is not in the DB
    for rom in scanned:
        if rom.crc32 not in db_crcs:
            report.orphan_roms.append((rom.filename, rom.crc32))

    # Status summary counts
    for entry in database:
        status = entry.status or "not_tested"
        report.status_counts[status] = (
            report.status_counts.get(status, 0) + 1
        )

    return report


# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------


def cmd_scan(args) -> int:
    """Handle the 'scan' subcommand."""
    path = Path(args.path)
    results: list[RomInfo] = []

    if not path.exists():
        print(f"Error: path not found: {path}", file=__import__("sys").stderr)
        return 1

    if path.is_file() and path.suffix.lower() == ".zip":
        results = scan_zip(str(path))
    elif path.is_file() and _is_bin_file(path.name):
        crc = compute_crc32(str(path))
        size = path.stat().st_size
        results = [RomInfo(path.name, crc, size, "local")]
    elif path.is_dir():
        results = scan_directory(str(path))
    else:
        print(
            f"Error: unsupported path: {path}",
            file=__import__("sys").stderr,
        )
        return 1

    for rom in results:
        print(f"{rom.filename} {rom.crc32}")
    return 0


def cmd_populate(args) -> int:
    """Handle the 'populate' subcommand."""
    import sys

    gamelist_path = Path(args.gamelist)
    if not gamelist_path.is_file():
        print(
            f"Error: Gamelist not found: {gamelist_path}",
            file=sys.stderr,
        )
        return 1

    # Scan all ROM sources
    roms = scan_all(args.roms_dir, args.bios_dir, args.tosec_dir)
    if not roms:
        print("Warning: no ROMs found", file=sys.stderr)

    # Parse gamelist
    gamelist = parse_gamelist(str(gamelist_path))

    # Deduplicate and merge
    merged = deduplicate_by_crc(roms)
    rows = merge_with_gamelist(merged, gamelist)

    # Write CSV
    write_csv(rows, args.output)
    print(f"Wrote {len(rows)} entries to {args.output}")
    return 0


def cmd_validate(args) -> int:
    """Handle the 'validate' subcommand."""
    import sys

    db_path = Path(args.database)
    if not db_path.is_file():
        print(
            f"Error: database not found: {db_path}",
            file=sys.stderr,
        )
        return 1

    # Load database and scan ROMs
    database = load_csv(str(db_path))
    scanned = scan_all(args.roms_dir, args.bios_dir, args.tosec_dir)

    # Validate
    report = validate(database, scanned)

    # Print report
    has_warnings = False

    if report.crc_mismatches:
        has_warnings = True
        print("CRC mismatches:")
        for fn, expected, actual in report.crc_mismatches:
            print(f"  {fn}: expected {expected}, got {actual}")

    if report.missing_roms:
        has_warnings = True
        print("Missing ROMs (in DB but not on disk):")
        for fn in report.missing_roms:
            print(f"  {fn}")

    if report.orphan_roms:
        has_warnings = True
        print("Orphan ROMs (on disk but not in DB):")
        for fn, crc in report.orphan_roms:
            print(f"  {fn} {crc}")

    # Status summary
    print("\nStatus summary:")
    for status, count in sorted(report.status_counts.items()):
        print(f"  {status}: {count}")

    if not has_warnings:
        print("\nDatabase is consistent with ROM collection.")

    return 2 if has_warnings else 0


def main() -> int:
    """CLI entry point with scan, populate, validate subcommands."""
    import argparse

    parser = argparse.ArgumentParser(
        description="ROM Compatibility Database Tool",
    )
    sub = parser.add_subparsers(dest="command")

    # --- scan ---
    p_scan = sub.add_parser(
        "scan",
        help="Compute CRC32 checksums for ROM files",
    )
    p_scan.add_argument(
        "path",
        help="Path to file, directory, or zip archive",
    )

    # --- populate ---
    p_pop = sub.add_parser(
        "populate",
        help="Generate compatibility.csv from ROM collection",
    )
    p_pop.add_argument(
        "--roms-dir", default="roms",
        help="Path to local ROM directory (default: roms)",
    )
    p_pop.add_argument(
        "--bios-dir", default="roms/BIOS",
        help="Path to BIOS directory (default: roms/BIOS)",
    )
    p_pop.add_argument(
        "--tosec-dir",
        default="roms/Magnavox_Odyssey_2_TOSEC_2012_04_23",
        help="Path to TOSEC directory",
    )
    p_pop.add_argument(
        "--gamelist", default="roms/Gamelist777.txt",
        help="Path to Gamelist777.txt",
    )
    p_pop.add_argument(
        "--output", default="doc/compatibility.csv",
        help="Output CSV path (default: doc/compatibility.csv)",
    )

    # --- validate ---
    p_val = sub.add_parser(
        "validate",
        help="Validate database against ROM files",
    )
    p_val.add_argument(
        "--database", default="doc/compatibility.csv",
        help="Path to compatibility.csv",
    )
    p_val.add_argument(
        "--roms-dir", default="roms",
        help="Path to local ROM directory (default: roms)",
    )
    p_val.add_argument(
        "--bios-dir", default="roms/BIOS",
        help="Path to BIOS directory (default: roms/BIOS)",
    )
    p_val.add_argument(
        "--tosec-dir",
        default="roms/Magnavox_Odyssey_2_TOSEC_2012_04_23",
        help="Path to TOSEC directory",
    )

    args = parser.parse_args()

    if args.command is None:
        parser.print_help()
        return 1

    if args.command == "scan":
        return cmd_scan(args)
    elif args.command == "populate":
        return cmd_populate(args)
    elif args.command == "validate":
        return cmd_validate(args)

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
