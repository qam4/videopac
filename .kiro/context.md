# Project Context and Guidelines

This document contains general instructions and guidelines for working on the Videopac emulator project.

## Git Commit Workflow

When making commits, always follow this process:

1. **Test First**: Run the test suite to ensure all tests pass
   ```bash
   ./build/videopac_tests
   ```
   **CRITICAL**: All tests MUST pass before committing. Never commit with failing tests.

2. **Stage Relevant Files**: Add only the files related to the current change
   ```bash
   git add <specific-files>
   ```
   - Do NOT use `git add .` or `git add -A` without reviewing
   - Stage files that are logically related to the same change

3. **Review Changes**: Check what will be committed
   ```bash
   git diff --staged
   ```
   - Review every line to ensure no unintended changes
   - Verify no debug code, commented code, or temporary changes are included

4. **Draft Commit Message**: Follow conventional commit format
   ```
   <type>: <short summary>
   
   <detailed description if needed>
   
   <footer with references if applicable>
   ```

5. **Commit Types**:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `style:` - Code style changes (formatting, no logic change)
   - `refactor:` - Code refactoring
   - `test:` - Adding or updating tests
   - `chore:` - Build process, dependencies, tooling
   - `perf:` - Performance improvements

6. **Commit Message Guidelines**:
   - Use imperative mood ("Add feature" not "Added feature")
   - Keep first line under 72 characters
   - Separate subject from body with blank line
   - Explain what and why, not how
   - Reference issue/task numbers when applicable

## Example Commit Messages

Good:
```
feat: implement Intel 8048 CPU core structure

Add CPU state structure with all registers, internal RAM, stack,
and I/O ports. Implement reset() and basic interface methods.

Implements task 3.1 from implementation plan.
```

```
test: add unit tests for memory bank switching

Verify correct behavior for 2KB, 4KB, and 8KB ROM cartridges
with proper bank selection via Port 1 pins.
```

```
chore: set up CMake build system with testing frameworks

Configure CMake to build core library, standalone executable,
and libretro core. Integrate Google Test and RapidCheck for
unit and property-based testing.

Completes task 1 from implementation plan.
```

Bad:
```
fixed stuff
```

```
WIP
```

```
Updated files
```

## Code Review Checklist

Before committing, verify:
- [ ] **All tests pass** (MANDATORY - never commit with failing tests)
- [ ] No compiler warnings
- [ ] Code follows C++17 standards
- [ ] No unused variables or parameters (use `(void)var` for intentional unused)
- [ ] No debug print statements
- [ ] No commented-out code
- [ ] Proper error handling
- [ ] Documentation updated if needed

## Build and Test Commands

```bash
# Configure build
cd build
cmake3 ..

# Build all targets
cmake3 --build .

# Run tests
./videopac_tests

# Run standalone emulator
./videopac <rom_file>
```
