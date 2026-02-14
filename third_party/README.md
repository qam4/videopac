# Third-Party Dependencies

This directory contains single-header libraries used by the Videopac emulator.

## Libraries

### miniz
- **Version**: 3.1.1
- **License**: MIT
- **Purpose**: ZIP file compression/decompression
- **Source**: https://github.com/richgel999/miniz
- **Installation**: Install via vcpkg (see below)

### stb_image_write
- **Version**: Latest from master branch
- **License**: Public Domain / MIT
- **Purpose**: Image file writing (PNG, BMP, TGA) for screenshots and thumbnails
- **Source**: https://github.com/nothings/stb
- **Files**: stb_image_write.h

## Installing Dependencies via vcpkg

### miniz

```bash
# Windows (MSVC)
vcpkg install miniz:x64-windows

# Windows (MinGW)
vcpkg install miniz:x64-mingw-dynamic --host-triplet=x64-mingw-dynamic

# Linux
vcpkg install miniz

# macOS
vcpkg install miniz
```

### SDL2_ttf

SDL2_ttf is an optional dependency for text rendering:

```bash
# Windows (MSVC)
vcpkg install sdl2-ttf:x64-windows

# Windows (MinGW)
vcpkg install sdl2-ttf:x64-mingw-dynamic --host-triplet=x64-mingw-dynamic

# Linux
sudo apt-get install libsdl2-ttf-dev

# macOS
brew install sdl2_ttf
```

If SDL2_ttf is not available, the emulator will fall back to basic SDL2 text rendering.

## Updating stb_image_write

To update stb_image_write, download the latest version:

```bash
curl -o third_party/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
```
