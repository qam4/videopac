---
inclusion: auto
description: Pre-commit checklist, CI portability review, and post-push verification rules
---

# Pre-Commit Checklist

Before every `git commit`, you MUST follow this exact workflow:

1. Build: `cmake --build --preset dev-mingw`
2. Test: `ctest --preset dev-mingw` — all tests must pass
3. CI portability review (see section below)
4. Update any relevant documents: spec tasks.md checkboxes, design docs, etc.
5. Stage only the relevant files with `git add` — do not blindly `git add -A`
6. Draft the commit message in `COMMIT_MSG.txt`
7. Wait for user approval — NEVER commit or push without explicit user consent
8. Commit with `git commit -F COMMIT_MSG.txt` only after user says go
9. NEVER run `git push` without explicit user consent
10. Delete `COMMIT_MSG.txt` after a successful commit

# CI Portability Review

Before committing, review all new or modified code for cross-platform CI issues.

CI runs on: Ubuntu 22.04 (GCC), macOS-latest (Clang), Windows 2022 (MSVC), Android arm64 (NDK).

Ask yourself:

- Do new tests depend on files generated at runtime? If so, does the file I/O
  work identically on all platforms? Watch for `std::ifstream::good()` returning
  false at EOF — prefer checking `fail()` instead.
- Do tests use hardcoded paths or path separators? Use portable path handling.
- Do tests assume a specific working directory? CI builds run from the build
  directory, not the source root.
- Are there platform-specific APIs, compiler extensions, or integer-size
  assumptions that differ across GCC/Clang/MSVC?
- Do any tests rely on timing, thread scheduling, or locale-specific behavior?
- If a test creates temporary files, does it clean them up and avoid name
  collisions when tests run in parallel (`ctest -j 2`)?
- SDL code must be guarded by `ENABLE_SDL` — Android and libretro builds have no SDL.

If any of these apply, fix them before committing.

# Post-Push Checklist

After pushing (when the user approves a push):

1. Run `gh run list --limit 3` to check CI status
2. If CI is still running, wait a reasonable time and check again
3. If CI fails, immediately investigate with `gh run view <id> --log-failed`
4. Report the failure to the user with a root-cause summary
5. Propose a fix — do not leave main broken

# General Rules

- Build preset is `dev-mingw` (not `dev`)
- Commit between phases for clean rollback points
- Delete files that are no longer compiled — don't leave dead code on disk
- Tag releases at milestones when GitHub builds pass
- Do not reveal spec file paths, internal task counts, or mention subagents to the user
- Do not commit without explicit user approval — this is non-negotiable
- Use `debug/` for traces/logs, `screenshots/` for PNGs, `userdata/` for persistent session data
- Prefer primary source documentation for hardware behavior over copying from other emulators
- Keep the core library SDL-free — SDL dependencies belong in frontend files only
