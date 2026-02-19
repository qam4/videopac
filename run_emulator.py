#!/usr/bin/env python3
"""
Videopac Emulator Launcher
Unified cross-platform script for running the emulator in different modes.
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


def get_executable_path():
    """Determine the emulator executable path based on platform."""
    system = platform.system()
    
    if system == "Windows":
        # Try dev-mingw first, then ci-win64
        paths = [
            Path("build/dev-mingw/videopac.exe"),
            Path("build/ci-win64/Release/videopac.exe"),
        ]
    else:
        # Linux/macOS
        paths = [
            Path("build/videopac"),
            Path("build/Release/videopac"),
        ]
    
    for path in paths:
        if path.exists():
            return str(path)
    
    print(f"Error: Could not find emulator executable. Tried:", file=sys.stderr)
    for path in paths:
        print(f"  - {path}", file=sys.stderr)
    sys.exit(1)


def clean_trace_log():
    """Remove old trace log file."""
    if Path("trace.log").exists():
        Path("trace.log").unlink()


def setup_screenshots_dir():
    """Clean and create screenshots directory."""
    screenshots_dir = Path("screenshots")
    if screenshots_dir.exists():
        shutil.rmtree(screenshots_dir)
    screenshots_dir.mkdir(exist_ok=True)


def convert_screenshots():
    """Convert screenshots using the conversion script."""
    convert_script = Path("convert_screenshots.sh")
    if convert_script.exists():
        print("Converting screenshots...")
        try:
            subprocess.run(["bash", str(convert_script)], check=True)
        except subprocess.CalledProcessError as e:
            print(f"Warning: Screenshot conversion failed: {e}", file=sys.stderr)
        except FileNotFoundError:
            print("Warning: bash not found, skipping screenshot conversion", file=sys.stderr)


def run_sdl_mode(exe_path, bios_path, rom_path, region):
    """Run emulator in SDL mode with debugger."""
    print("Running in SDL mode...")
    print()
    print("Note: For Course de Voitures ROM:")
    print("  - Press '1' for Game 1: Course de Voitures (road racing)")
    print("  - Press '2' for Game 2: Autodrome (top-down circuit racing)")
    print()
    
    cmd = [
        exe_path,
        "--region", region,
        "--debug",
        "--trace",
        "--bios", bios_path,
        rom_path
    ]
    
    subprocess.run(cmd)


def run_headless_mode(exe_path, bios_path, rom_path, region):
    """Run emulator in headless mode with screenshot capture."""
    print("Running in HEADLESS mode...")
    print("Note: Pressing '1' to select Game 1 (Course de Voitures)")
    print("Watching memory address 0x3F for writes")
    print()
    
    setup_screenshots_dir()
    
    cmd = [
        exe_path,
        "--headless",
        "--screenshot", "1",
        "--frames", "200",
        "--press-key", "1", "5",   # Press '1' at frame 5 to start game selection
        "--press-key", "1", "10",  # Press '1' at frame 10 to select game level 1
        "--debug",
        "--trace",
        "--bios", bios_path,
        rom_path
    ]
    
    print("Running emulator (output will appear below)...")
    print("=" * 60)
    result = subprocess.run(cmd)
    print("=" * 60)
    print(f"Emulator exited with code: {result.returncode}")
    print()
    
    convert_screenshots()


def run_dcv_mode(exe_path, bios_path, rom_path, region):
    """Run emulator in DCV mode (remote desktop optimized)."""
    print("Running in DCV mode (remote desktop)...")
    
    # Set environment variables for remote desktop
    env = os.environ.copy()
    env["DISPLAY"] = ":0"
    env["SDL_RENDER_DRIVER"] = "software"
    env["SDL_AUDIODRIVER"] = "dummy"  # Disable audio to avoid errors
    
    cmd = [
        exe_path,
        "--region", region,
        "--debug",
        "--bios", bios_path,
        rom_path
    ]
    
    subprocess.run(cmd, env=env)


def main():
    parser = argparse.ArgumentParser(
        description="Videopac Emulator Launcher",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                                    # SDL mode with default ROM
  %(prog)s sdl                                # SDL mode with default ROM
  %(prog)s headless                           # Headless mode with default ROM
  %(prog)s debug-scroll                       # Debug scroll issue
  %(prog)s dcv                                # DCV mode with default ROM
  %(prog)s sdl "roms/game.bin"                # SDL mode with custom ROM
  %(prog)s headless "roms/game.bin"           # Headless mode with custom ROM
  %(prog)s dcv "roms/game.bin"                # DCV mode with custom ROM
        """
    )
    
    parser.add_argument(
        "mode",
        nargs="?",
        default="sdl",
        choices=["sdl", "headless", "hl", "debug-scroll", "dcv"],
        help="Emulator mode: sdl (default), headless/hl, debug-scroll, or dcv"
    )
    
    parser.add_argument(
        "rom",
        nargs="?",
        default="roms/Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR).bin",
        # default="roms/Satellite Attack (1981)(Philips)(EU).bin",
        help="Path to ROM file (default: Course de Voitures / Autodrome)"
    )
    
    parser.add_argument(
        "--bios",
        default="roms/Philips C52 BIOS (19xx)(Philips)(FR).bin",
        help="Path to BIOS file (default: French C52 BIOS)"
    )
    
    parser.add_argument(
        "--region",
        default="france",
        choices=["usa", "europe", "france"],
        help="Region setting (default: france)"
    )
    
    args = parser.parse_args()
    
    # Get executable path
    exe_path = get_executable_path()
    
    # Clean up old files
    clean_trace_log()
    
    # Normalize mode
    mode = args.mode.lower()
    if mode == "hl":
        mode = "headless"
    
    # Run appropriate mode
    if mode == "headless":
        run_headless_mode(exe_path, args.bios, args.rom, args.region)
    elif mode == "debug-scroll":
        run_debug_scroll_mode(exe_path, args.bios, args.rom, args.region)
    elif mode == "dcv":
        run_dcv_mode(exe_path, args.bios, args.rom, args.region)
    else:  # sdl
        run_sdl_mode(exe_path, args.bios, args.rom, args.region)


if __name__ == "__main__":
    main()
