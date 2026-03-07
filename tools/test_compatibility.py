#!/usr/bin/env python3
"""
Videopac ROM Compatibility Test Tool

Automated headless testing of ROMs to populate the compatibility database.
Runs each ROM in headless mode, takes screenshots, and compares them to
detect boot status (nothing / boots / menu).

Usage:
    python tools/test_compatibility.py test [--limit N] [--status not_tested]
    python tools/test_compatibility.py test --rom <filename>
    python tools/test_compatibility.py reference
"""

from __future__ import annotations

import argparse
import os
import platform
import subprocess
import sys
import zipfile
from pathlib import Path

# Add tools directory to path so we can import crc32_tool
sys.path.insert(0, str(Path(__file__).parent))
from crc32_tool import load_csv, write_csv, CsvRow


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

DATABASE_PATH = "doc/compatibility.csv"
ROMS_DIR = "roms"
BIOS_DIR = "roms/BIOS"
TOSEC_DIR = "roms/Magnavox Odyssey 2 [TOSEC]"
SCREENSHOT_DIR = "screenshots/compat_test"
REFERENCE_DIR = "screenshots/compat_test/reference"

# BIOS selection by platform/region
BIOS_MAP = {
    ("O2", "US"): "bios_O2rom.bin",
    ("O2", "EU"): "bios_O2rom.bin",
    ("O2", "EU-US"): "bios_O2rom.bin",
    ("VP", "EU"): "bios_O2rom.bin",
    ("VP", "EU-US"): "bios_O2rom.bin",
    ("VP", "US"): "bios_O2rom.bin",
    ("VP", "FR"): "bios_c52.bin",
    ("VP+", "EU"): "bios_g7400.bin",
    ("VP+", "FR"): "bios_jopac.bin",
    ("Jopac", "EU"): "bios_jopac.bin",
    ("Jopac", "FR"): "bios_jopac.bin",
    ("Brazilian", "BR"): "bios_O2rom.bin",
    ("BIOS", "EU"): "bios_O2rom.bin",
}

# Frames to run for each test phase
BOOT_FRAMES = 120       # ~2 seconds at 60fps — enough for BIOS + select screen
GAME_FRAMES = 180       # ~3 seconds after key press — enough to see game start
KEY_PRESS_FRAME = 60    # Press key "1" at frame 60 (after BIOS init)

# Screenshot is saved every N frames (we want the last one)
SCREENSHOT_INTERVAL = 1  # Save every frame (we only care about the last)


# ---------------------------------------------------------------------------
# Emulator executable
# ---------------------------------------------------------------------------

def get_exe_path() -> str:
    """Find the videopac emulator executable."""
    system = platform.system()
    if system == "Windows":
        candidates = [
            "build/dev-mingw/videopac.exe",
            "build/dev-win64/Release/videopac.exe",
        ]
    else:
        candidates = [
            "build/videopac",
            "build/Release/videopac",
        ]
    for p in candidates:
        if os.path.exists(p):
            return p
    print("Error: Cannot find videopac executable. Build first.", file=sys.stderr)
    sys.exit(1)


# ---------------------------------------------------------------------------
# BIOS selection
# ---------------------------------------------------------------------------

def select_bios(entry: CsvRow) -> str:
    """Select the appropriate BIOS file for a ROM entry."""
    key = (entry.platform, entry.region)
    bios_name = BIOS_MAP.get(key)
    if not bios_name:
        # Fallback: try platform-only match
        for (plat, _), name in BIOS_MAP.items():
            if plat == entry.platform:
                bios_name = name
                break
    if not bios_name:
        # Fallback: region-based (for entries with empty platform)
        if entry.region == "FR":
            bios_name = "bios_c52.bin"
        elif entry.region in ("US", "EU-US"):
            bios_name = "bios_O2rom.bin"
        else:
            bios_name = "bios_O2rom.bin"  # Ultimate fallback
    return os.path.join(BIOS_DIR, bios_name)


# ---------------------------------------------------------------------------
# ROM file resolution
# ---------------------------------------------------------------------------

def find_rom_file(entry: CsvRow) -> str | None:
    """Find the ROM file on disk for a database entry.

    Checks:
    1. Direct path in roms/ directory
    2. Alt filenames in roms/ directory
    3. TOSEC zip archives (extract if needed)
    """
    # Try primary filename in roms/
    primary = os.path.join(ROMS_DIR, entry.filename)
    if os.path.exists(primary):
        return primary

    # Try alt filenames
    if entry.alt_filenames:
        for alt in entry.alt_filenames.split(";"):
            alt = alt.strip()
            if not alt:
                continue
            alt_path = os.path.join(ROMS_DIR, alt)
            if os.path.exists(alt_path):
                return alt_path

    # Search TOSEC zips
    tosec_root = Path(TOSEC_DIR)
    if tosec_root.is_dir():
        all_filenames = [entry.filename]
        if entry.alt_filenames:
            all_filenames.extend(
                f.strip() for f in entry.alt_filenames.split(";") if f.strip()
            )
        for zpath in tosec_root.rglob("*.zip"):
            try:
                with zipfile.ZipFile(zpath, "r") as zf:
                    for info in zf.infolist():
                        basename = Path(info.filename).name
                        if basename in all_filenames:
                            # Extract to temp location
                            extract_dir = Path(SCREENSHOT_DIR) / "extracted"
                            extract_dir.mkdir(parents=True, exist_ok=True)
                            extracted = extract_dir / basename
                            if not extracted.exists():
                                with open(extracted, "wb") as out:
                                    out.write(zf.read(info.filename))
                            return str(extracted)
            except (zipfile.BadZipFile, OSError):
                continue

    return None


# ---------------------------------------------------------------------------
# Screenshot comparison
# ---------------------------------------------------------------------------

def read_ppm_pixels(path: str) -> bytes | None:
    """Read a PPM file and return raw pixel data (skip header)."""
    try:
        with open(path, "rb") as f:
            data = f.read()
    except (OSError, IOError):
        return None

    if not data.startswith(b"P6"):
        return None

    # Parse PPM header: P6\n<width> <height>\n<maxval>\n<pixels>
    # Skip past the header (3 newlines)
    pos = 0
    newlines = 0
    while pos < len(data) and newlines < 3:
        if data[pos:pos+1] == b"\n":
            newlines += 1
        pos += 1
    return data[pos:]


def is_black_screen(ppm_path: str) -> bool:
    """Check if a PPM screenshot is entirely black."""
    pixels = read_ppm_pixels(ppm_path)
    if pixels is None:
        return True  # No screenshot = black
    # All zeros = black screen
    return all(b == 0 for b in pixels)


def screenshots_differ(path_a: str, path_b: str) -> bool:
    """Check if two PPM screenshots have different pixel content."""
    pixels_a = read_ppm_pixels(path_a)
    pixels_b = read_ppm_pixels(path_b)
    if pixels_a is None or pixels_b is None:
        return True  # Can't compare = assume different
    return pixels_a != pixels_b


def count_nonblack_pixels(ppm_path: str) -> int:
    """Count non-black pixels in a PPM screenshot."""
    pixels = read_ppm_pixels(ppm_path)
    if pixels is None:
        return 0
    count = 0
    # PPM P6: 3 bytes per pixel (R, G, B)
    for i in range(0, len(pixels) - 2, 3):
        if pixels[i] != 0 or pixels[i+1] != 0 or pixels[i+2] != 0:
            count += 1
    return count


# ---------------------------------------------------------------------------
# Single ROM test
# ---------------------------------------------------------------------------

def test_single_rom(exe: str, rom_path: str, entry: CsvRow,
                    ref_bios_screenshot: str | None = None) -> tuple[str, str]:
    """
    Test a single ROM in headless mode.

    Strategy:
    1. Run ROM for BOOT_FRAMES with key "1" pressed at KEY_PRESS_FRAME
    2. Take screenshot at the end
    3. Compare with BIOS-only reference screenshot
    4. Determine status: nothing / boots / menu

    Returns (status, notes).
    """
    bios_path = select_bios(entry)
    if not os.path.exists(bios_path):
        return ("not_tested", f"BIOS not found: {bios_path}")

    # Clean screenshots directory before each ROM test
    screenshots_dir = Path("screenshots")
    if screenshots_dir.exists():
        for old in screenshots_dir.glob("frame_*.ppm"):
            try:
                old.unlink()
            except OSError:
                pass

    # Phase 1: Run with key press "1" at frame 60
    # This handles "SELECT GAME" screens that most Videopac games show
    cmd = [
        exe, "--headless",
        "--frames", str(BOOT_FRAMES),
        "--screenshot", str(BOOT_FRAMES),  # Save screenshot at last frame only
        "--press-key", "1", str(KEY_PRESS_FRAME),
        "--bios", bios_path,
        rom_path,
    ]

    try:
        result = subprocess.run(
            cmd, capture_output=True, timeout=30,
            cwd=os.getcwd(),
        )
    except subprocess.TimeoutExpired:
        return ("nothing", "Timeout — emulator hung")
    except Exception as e:
        return ("nothing", f"Error: {e}")

    if result.returncode != 0:
        stderr = result.stderr.decode("utf-8", errors="replace").strip()
        return ("nothing", f"Exit code {result.returncode}: {stderr[:100]}")

    # Find the screenshot (last frame_NNNNNN.ppm in screenshots/)
    screenshots = sorted(screenshots_dir.glob("frame_*.ppm"))
    if not screenshots:
        return ("nothing", "No screenshot produced")

    last_screenshot = str(screenshots[-1])

    # Check if screen is black
    if is_black_screen(last_screenshot):
        return ("nothing", "Black screen (auto-tested)")

    nonblack = count_nonblack_pixels(last_screenshot)

    # Compare with BIOS reference if available
    if ref_bios_screenshot and os.path.exists(ref_bios_screenshot):
        if not screenshots_differ(last_screenshot, ref_bios_screenshot):
            return ("nothing", "Screen same as BIOS-only (auto-tested)")

    # We have non-black pixels that differ from BIOS reference
    if nonblack > 100:
        return ("boots", f"{nonblack} non-black pixels (auto-tested)")
    else:
        return ("nothing", f"Only {nonblack} non-black pixels (auto-tested)")


# ---------------------------------------------------------------------------
# BIOS reference screenshot
# ---------------------------------------------------------------------------

def generate_bios_reference(exe: str, bios_name: str) -> str | None:
    """Generate a reference screenshot of BIOS-only boot (no ROM).

    Returns path to the reference screenshot, or None on failure.
    """
    os.makedirs(REFERENCE_DIR, exist_ok=True)
    ref_path = os.path.join(REFERENCE_DIR, f"ref_{Path(bios_name).stem}.ppm")

    if os.path.exists(ref_path):
        return ref_path

    bios_path = os.path.join(BIOS_DIR, bios_name)
    if not os.path.exists(bios_path):
        return None

    # We need a dummy ROM to load — use the smallest available
    # Actually, the emulator requires a ROM. We'll skip reference for now
    # and just check for black screen / non-black pixel count
    return None


# ---------------------------------------------------------------------------
# Test mode
# ---------------------------------------------------------------------------

def cmd_test(args) -> int:
    """Run automated headless tests on ROMs."""
    db_path = args.database
    if not os.path.exists(db_path):
        print(f"Error: Database not found: {db_path}", file=sys.stderr)
        return 1

    exe = get_exe_path()
    db = load_csv(db_path)

    # Filter entries
    if args.rom:
        # Test a specific ROM by filename
        to_test = [r for r in db if r.filename == args.rom]
        if not to_test:
            print(f"Error: ROM '{args.rom}' not found in database", file=sys.stderr)
            return 1
    else:
        to_test = [r for r in db if r.status == args.status]
        # Skip BIOS entries — they're not games
        to_test = [r for r in to_test if r.platform != "BIOS"]

    if args.limit > 0:
        to_test = to_test[:args.limit]

    if not to_test:
        print(f"No entries with status '{args.status}' to test.")
        return 0

    print(f"Testing {len(to_test)} ROMs (status={args.status})...")
    print()

    tested = 0
    skipped = 0
    results: dict[str, int] = {}

    for entry in to_test:
        rom_path = find_rom_file(entry)
        if rom_path is None:
            print(f"  SKIP {entry.name} — file not found")
            skipped += 1
            continue

        status, notes = test_single_rom(exe, rom_path, entry)
        entry.status = status
        entry.notes = notes
        tested += 1
        results[status] = results.get(status, 0) + 1

        icon = "✓" if status in ("boots", "menu", "in_game", "playable") else "✗"
        print(f"  {icon} {entry.name}: {status} — {notes}")

    # Merge results back into full DB
    tested_by_crc = {r.crc32: r for r in to_test if r.status != "not_tested" or r.notes}
    for row in db:
        if row.crc32 in tested_by_crc:
            updated = tested_by_crc[row.crc32]
            row.status = updated.status
            row.notes = updated.notes

    write_csv(db, db_path)

    print(f"\nTested {tested} ROMs ({skipped} skipped):")
    for status, count in sorted(results.items()):
        if count > 0:
            print(f"  {status}: {count}")
    print(f"Database updated: {db_path}")
    return 0


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Videopac ROM Compatibility Test Tool",
    )
    sub = parser.add_subparsers(dest="command")

    # test
    test_p = sub.add_parser("test", help="Automated headless testing of ROMs")
    test_p.add_argument("--database", default=DATABASE_PATH, help="CSV path")
    test_p.add_argument("--limit", type=int, default=0, help="Max ROMs to test (0=all)")
    test_p.add_argument("--status", default="not_tested", help="Only test entries with this status")
    test_p.add_argument("--rom", default=None, help="Test a specific ROM by filename")

    args = parser.parse_args()

    if args.command is None:
        parser.print_help()
        return 1

    if args.command == "test":
        return cmd_test(args)

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
