---
inclusion: auto
description: Videopac-specific build commands and rules
---

# Build & Test Commands

1. Build: `cmake --build --preset dev-mingw`
2. Test: `ctest --preset dev-mingw`

# CI Platforms

Ubuntu 22.04 (GCC), macOS-latest (Clang), Windows 2022 (MSVC), Android arm64 (NDK).

SDL code must be guarded by `ENABLE_SDL` — Android and libretro builds have no SDL.

# Version Sync Check

At every push (not just at tagging time):

1. Run `git describe --tags --abbrev=0` to find the latest tag
2. Read `videopac_libretro.info` and verify `display_version` matches the latest tag (without the `v` prefix)
3. If they don't match, update `display_version` in the info file and include it in the commit

When tagging a new release (`git tag vX.Y.Z`):

1. Verify `display_version` matches the new tag (without the `v` prefix)
2. If they don't match, update `display_version` before tagging
3. Amend the commit if needed, then create the tag on the amended commit

# Project Rules

- Build preset is `dev-mingw` (not `dev`)
- Use `debug/` for traces/logs, `screenshots/` for PNGs, `userdata/` for persistent session data
- Prefer primary source documentation for hardware behavior over copying from other emulators
- Keep the core library SDL-free — SDL dependencies belong in frontend files only
