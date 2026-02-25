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
        # Try build root first, then dev-mingw, then dev-win64
        paths = [
            # Path("build/videopac.exe"),
            Path("build/dev-mingw/videopac.exe"),
            Path("build/dev-win64/Release/videopac.exe"),
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
    """Remove old trace log files."""
    for trace_file in ["trace_cpu.log", "trace_vdc.log"]:
        if Path(trace_file).exists():
            try:
                Path(trace_file).unlink()
            except PermissionError:
                # File is locked by another process, skip deletion
                print(f"Warning: {trace_file} is locked by another process, skipping cleanup", file=sys.stderr)


def setup_screenshots_dir():
    """Clean and create screenshots directory."""
    screenshots_dir = Path("screenshots")
    if screenshots_dir.exists():
        # Try to remove all files in the directory
        try:
            for item in screenshots_dir.iterdir():
                try:
                    if item.is_file():
                        item.unlink()
                    elif item.is_dir():
                        shutil.rmtree(item)
                except Exception as e:
                    print(f"Warning: Could not remove {item}: {e}", file=sys.stderr)
        except Exception as e:
            print(f"Warning: screenshots directory cleanup failed ({e}), continuing anyway", file=sys.stderr)
    else:
        screenshots_dir.mkdir(exist_ok=True)


def convert_screenshots():
    """Convert PPM screenshots to PNG for easier viewing."""
    screenshots_dir = Path("screenshots")
    if not screenshots_dir.exists():
        return
    
    ppm_files = list(screenshots_dir.glob("*.ppm"))
    if not ppm_files:
        return
    
    print("Converting screenshots...")
    try:
        from PIL import Image
        for ppm_file in ppm_files:
            png_file = ppm_file.with_suffix('.png')
            img = Image.open(ppm_file)
            img.save(png_file)
            print(f"  {ppm_file.name} -> {png_file.name}")
        print("Conversion complete!")
    except ImportError:
        print("Warning: PIL/Pillow not installed. Install with: pip install Pillow", file=sys.stderr)
    except Exception as e:
        print(f"Warning: Screenshot conversion failed: {e}", file=sys.stderr)


def run_sdl_mode(exe_path, bios_path, rom_path, extra_args):
    """Run emulator in SDL mode with debugger."""
    print("Running in SDL mode...")
    print()
    
    cmd = [
        exe_path,
        "--debug",
        "--trace",
        "--vdc-trace",
        "--bios", bios_path,
        rom_path
    ]
    
    # Add any extra arguments (breakpoints, watch expressions, region, etc.)
    cmd.extend(extra_args)
    
    subprocess.run(cmd)


def run_headless_mode(exe_path, bios_path, rom_path, extra_args, no_input=False, frames=None):
    """Run emulator in headless mode with screenshot capture."""
    print("Running in HEADLESS mode...")
    print()
    
    setup_screenshots_dir()
    
    # Use provided frames or defaults
    if frames is None:
        frames = 20
    
    if no_input:
        # No input mode - only basic setup, no key/joystick presses
        cmd = [
            exe_path,
            "--headless",
            "--screenshot", "1",
            "--frames", str(frames),
            "--debug",
            "--trace",
            "--vdc-trace",
            "--bios", bios_path,
            rom_path
        ]
    else:
        # Normal mode with input
        cmd = [
            exe_path,
            "--headless",
            "--screenshot", "1",
            "--frames", str(frames),
            "--press-joystick", "2", "0", "20", str(frames),  # Press joystick 2 UP at frame 20
            "--press-key", "1", "5", "5",   # Press '1' at frame 5 for 5 frames (title screen)
            "--press-key", "1", "12", "5",  # Press '1' at frame 12 for 5 frames (select game)
            "--debug",
            "--trace",
            "region=france",
            "--vdc-trace",
            "--bios", bios_path,
            rom_path
        ]
    
    # Add any extra arguments (breakpoints, watch expressions, region, etc.)
    cmd.extend(extra_args)
    
    print("Running emulator (output will appear below)...")
    print("Command:", " ".join(cmd))
    print("=" * 60)
    result = subprocess.run(cmd)
    print("=" * 60)
    print(f"Emulator exited with code: {result.returncode}")
    print()
    
    convert_screenshots()


def run_dcv_mode(exe_path, bios_path, rom_path, extra_args):
    """Run emulator in DCV mode (remote desktop optimized)."""
    print("Running in DCV mode (remote desktop)...")
    
    # Set environment variables for remote desktop
    env = os.environ.copy()
    env["DISPLAY"] = ":0"
    env["SDL_RENDER_DRIVER"] = "software"
    env["SDL_AUDIODRIVER"] = "dummy"  # Disable audio to avoid errors
    
    cmd = [
        exe_path,
        "--debug",
        "--bios", bios_path,
        rom_path
    ]
    
    # Add any extra arguments (breakpoints, watch expressions, region, etc.)
    cmd.extend(extra_args)
    
    subprocess.run(cmd, env=env)
    cmd.extend(extra_args)
    
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
  %(prog)s dcv                                # DCV mode with default ROM
  %(prog)s sdl "roms/game.bin"                # SDL mode with custom ROM
  %(prog)s headless "roms/game.bin"           # Headless mode with custom ROM
  
  # With debugger options:
  %(prog)s sdl --watch "RAM[0x30]!=0"         # Watch expression
  %(prog)s sdl --break 0x495                  # Breakpoint at address
  %(prog)s sdl --break 0x495 --condition "A==0xFF"  # Conditional breakpoint
        """
    )
    
    parser.add_argument(
        "mode",
        nargs="?",
        default="sdl",
        choices=["sdl", "headless", "hl", "dcv"],
        help="Emulator mode: sdl (default), headless/hl, or dcv"
    )
    
    parser.add_argument(
        "rom",
        nargs="?",
        # default="roms/Magnavox Odyssey 2 [TOSEC]/Magnavox Odyssey2 - Games (TOSEC-v2011-02-22_CM)/Killer Bees (1983)(Philips)(US).zip",
        default="roms/ROMS/o2_47.bin",
        help="Path to ROM file (default: Killer Bees)"
    )
    
    parser.add_argument(
        "--bios",
        default="roms/Philips C52 BIOS (19xx)(Philips)(FR).bin",
        help="Path to BIOS file (default: French C52 BIOS)"
    )
    
    parser.add_argument(
        "--region",
        default=None,
        choices=["usa", "europe", "france"],
        help="Region setting (default: usa if not specified)"
    )
    
    parser.add_argument(
        "--no-input",
        action="store_true",
        help="Headless mode: run without any input (for baseline testing)"
    )
    
    parser.add_argument(
        "--frames",
        type=int,
        default=None,
        help="Number of frames to run in headless mode (default: 200 for no-input, 360 otherwise)"
    )
    
    # Debugger options
    parser.add_argument(
        "--watch",
        action="append",
        dest="watch_expressions",
        help="Watch expression (e.g., 'RAM[0x30]!=0')"
    )
    
    parser.add_argument(
        "--break",
        action="append",
        dest="breakpoints",
        help="Breakpoint address (e.g., 0x495)"
    )
    
    parser.add_argument(
        "--condition",
        action="append",
        dest="conditions",
        help="Condition for previous breakpoint (e.g., 'A==0xFF')"
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
    
    # Build extra arguments for debugger
    extra_args = []
    
    # Add region if specified (otherwise use emulator default)
    if args.region:
        extra_args.extend(["--region", args.region])
    
    # Add watch expressions
    if args.watch_expressions:
        for watch in args.watch_expressions:
            extra_args.extend(["--watch", watch])
    
    # Add breakpoints with optional conditions
    if args.breakpoints:
        for i, bp in enumerate(args.breakpoints):
            extra_args.extend(["--break", bp])
            # If there's a corresponding condition, add it
            if args.conditions and i < len(args.conditions):
                extra_args.extend(["--condition", args.conditions[i]])
    
    # Run appropriate mode
    if mode == "headless":
        run_headless_mode(exe_path, args.bios, args.rom, extra_args, no_input=args.no_input, frames=args.frames)
    elif mode == "dcv":
        run_dcv_mode(exe_path, args.bios, args.rom, extra_args)
    else:  # sdl
        run_sdl_mode(exe_path, args.bios, args.rom, extra_args)


if __name__ == "__main__":
    main()
