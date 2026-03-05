"""Smoke tests for tasks 5.1, 5.3, and 6.1."""
import sys
import os
import tempfile

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "tools"))

from crc32_tool import (
    RomInfo, CsvRow, MergedEntry,
    deduplicate_by_crc, merge_with_gamelist, write_csv, load_csv,
    detect_platform, detect_region,
)


def test_deduplicate_by_crc_basic():
    roms = [
        RomInfo("vp_01.bin", "AABBCCDD", 2048, "local"),
        RomInfo(
            "Race - Spin-out (1978)(Philips)(EU).bin",
            "AABBCCDD", 2048, "tosec",
        ),
        RomInfo("vp_02.bin", "11223344", 4096, "local"),
        RomInfo("unique_tosec.bin", "DEADBEEF", 1024, "tosec"),
    ]
    merged = deduplicate_by_crc(roms)
    assert len(merged) == 3
    assert merged["AABBCCDD"].primary_filename == "vp_01.bin"
    assert merged["AABBCCDD"].primary_source == "local"
    assert len(merged["AABBCCDD"].all_filenames) == 2


def test_deduplicate_prefers_local_over_tosec():
    roms = [
        RomInfo("tosec.bin", "AAAA0000", 100, "tosec"),
        RomInfo("local.bin", "AAAA0000", 100, "local"),
    ]
    merged = deduplicate_by_crc(roms)
    assert merged["AAAA0000"].primary_filename == "local.bin"
    assert merged["AAAA0000"].primary_source == "local"


def test_deduplicate_warns_on_same_source_dups():
    import warnings as w
    roms = [
        RomInfo("a.bin", "AAAA0000", 100, "local"),
        RomInfo("b.bin", "AAAA0000", 100, "local"),
    ]
    with w.catch_warnings(record=True) as caught:
        w.simplefilter("always")
        merged = deduplicate_by_crc(roms)
    assert len(merged) == 1
    assert any("Duplicate CRC32" in str(c.message) for c in caught)


def test_merge_with_gamelist_name_resolution():
    merged = {
        "AABBCCDD": MergedEntry(
            crc32="AABBCCDD",
            primary_filename="vp_01.bin",
            all_filenames=[
                "vp_01.bin",
                "Race - Spin-out (1978)(Philips)(EU).bin",
            ],
            primary_source="local",
            size=2048,
        ),
        "11223344": MergedEntry(
            crc32="11223344",
            primary_filename="vp_02.bin",
            all_filenames=["vp_02.bin"],
            primary_source="local",
            size=4096,
        ),
        "DEADBEEF": MergedEntry(
            crc32="DEADBEEF",
            primary_filename="Unique Game (1980)(Magnavox)(US).bin",
            all_filenames=["Unique Game (1980)(Magnavox)(US).bin"],
            primary_source="tosec",
            size=1024,
        ),
        "CAFEBABE": MergedEntry(
            crc32="CAFEBABE",
            primary_filename="unknown_local.bin",
            all_filenames=["unknown_local.bin"],
            primary_source="local",
            size=512,
        ),
    }
    gamelist = {
        "vp_01": "Race/Spin-out/Cryptogram",
        "vp_02": "Pick Axe Pete",
    }
    rows = merge_with_gamelist(merged, gamelist)
    by_crc = {r.crc32: r for r in rows}

    # Gamelist match
    assert by_crc["AABBCCDD"].name == "Race/Spin-out/Cryptogram"
    assert "Race - Spin-out" in by_crc["AABBCCDD"].alt_filenames

    # TOSEC-only: stem as name
    assert by_crc["DEADBEEF"].name == (
        "Unique Game (1980)(Magnavox)(US)"
    )

    # Local, no gamelist: filename as name
    assert by_crc["CAFEBABE"].name == "unknown_local.bin"

    # All entries have not_tested status
    for r in rows:
        assert r.status == "not_tested"


def test_write_csv_and_load_csv_roundtrip():
    rows = [
        CsvRow("AABB", "Game A", "a.bin", 100, "VP", "EU",
               "not_tested", "alt.bin", ""),
        CsvRow("CCDD", "Game B", "b.bin", 200, "O2", "US",
               "not_tested", "", "some notes"),
    ]
    with tempfile.NamedTemporaryFile(
        suffix=".csv", delete=False
    ) as f:
        tmp = f.name
    try:
        write_csv(rows, tmp)
        loaded = load_csv(tmp)
        assert len(loaded) == 2
        by_crc = {r.crc32: r for r in loaded}
        assert by_crc["AABB"].name == "Game A"
        assert by_crc["AABB"].size == 100
        assert by_crc["AABB"].alt_filenames == "alt.bin"
        assert by_crc["CCDD"].notes == "some notes"
    finally:
        os.unlink(tmp)


def test_csv_special_characters():
    row = CsvRow(
        "FFFF0000", 'Game with, commas', "test.bin",
        512, "VP", "EU", "not_tested", "",
        'Has "quotes" and, commas',
    )
    with tempfile.NamedTemporaryFile(
        suffix=".csv", delete=False
    ) as f:
        tmp = f.name
    try:
        write_csv([row], tmp)
        loaded = load_csv(tmp)
        assert len(loaded) == 1
        assert loaded[0].name == "Game with, commas"
        assert loaded[0].notes == 'Has "quotes" and, commas'
    finally:
        os.unlink(tmp)


def test_csv_sorting():
    rows = [
        CsvRow("AA", "Zebra", "z.bin", 100, "VP", "EU",
               "not_tested", "", ""),
        CsvRow("BB", "Alpha", "a.bin", 100, "VP", "EU",
               "not_tested", "", ""),
        CsvRow("CC", "Bios1", "bios_1.bin", 100, "BIOS", "EU",
               "not_tested", "", ""),
        CsvRow("DD", "Game", "o2_1.bin", 100, "O2", "US",
               "not_tested", "", ""),
    ]
    with tempfile.NamedTemporaryFile(
        suffix=".csv", delete=False
    ) as f:
        tmp = f.name
    try:
        write_csv(rows, tmp)
        loaded = load_csv(tmp)
        platforms = [r.platform for r in loaded]
        assert platforms == ["BIOS", "O2", "VP", "VP"]
        vp_names = [r.name for r in loaded if r.platform == "VP"]
        assert vp_names == ["Alpha", "Zebra"]
    finally:
        os.unlink(tmp)


def test_load_csv_warns_on_malformed():
    import warnings as w
    content = (
        "crc32,name,filename,size,platform,region,status,"
        "alt_filenames,notes\n"
        "AA,Good,g.bin,100,VP,EU,not_tested,,\n"
        "BB,Bad row only 3 fields,missing\n"
        "CC,Also Good,g2.bin,200,O2,US,not_tested,,\n"
    )
    with tempfile.NamedTemporaryFile(
        suffix=".csv", delete=False, mode="w"
    ) as f:
        f.write(content)
        tmp = f.name
    try:
        with w.catch_warnings(record=True) as caught:
            w.simplefilter("always")
            loaded = load_csv(tmp)
        assert len(loaded) == 2
        assert any("Malformed row" in str(c.message) for c in caught)
    finally:
        os.unlink(tmp)
