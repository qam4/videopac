# Select Game Routine Analysis

## Overview
The `select_game` routine at 0x2C3 is called by cartridge code to display a game selection menu and wait for user input.

## Flow

### 1. Initialization (0x2C3-0x2D4)
```
0x2C3: MOV R7,#0xFF          ; Initialize R7
0x2C5: SEL RB1               ; Select high register bank
0x2C6: CALL reset            ; Reset the machine
0x2C8: MOV R1,#0xF2          ; R1 points to text data at 0x2F2
0x2CA: MOV R0,#0x10          ; R0 points to VDC character register
0x2CC: MOV R2,#0x0B          ; 11 characters to display
0x2CE: MOV R3,#0x28          ; X position = 40
0x2D0: MOV R4,#0x70          ; Y position = 112
0x2D2: MOV R6,#0x06          ; Color = 6
0x2D4: CALL display_off      ; Turn off display
```

### 2. Display Text Loop (0x2D6-0x2DE)
```
loc_02d6:
0x2D6: MOV A,R1              ; Get text pointer
0x2D7: MOVP A,@A             ; Read character from ROM table at 0x2F2
0x2D8: MOV R5,A              ; Character to R5
0x2D9: CALL character_write  ; Write character to VDC
0x2DB: INC R1                ; Next character
0x2DC: INC R6                ; Increment X position
0x2DD: INC R6                ; (by 2)
0x2DE: DJNZ R2,loc_02d6      ; Loop for all 11 characters
```

The text at 0x2F2 is "QUEL JEU?" (French) or "SELECT GAME" (US BIOS).

### 3. Wait for Input (0x2E0-0x2E8)
```
0x2E0: CALL display_on       ; Turn display back on
0x2E2: MOV A,#0x4A           ; Tune data pointer
0x2E4: CALL start_tune       ; Play selection tune
0x2E6: CALL get_keystroke    ; **BLOCKS HERE** waiting for key
0x2E8: MOV R1,A              ; Save keystroke result
```

### 4. Cleanup and Return (0x2E9-0x2F0)
```
0x2E9: CALL display_off      ; Turn off display
0x2EB: CALL clear_all_characters  ; Clear screen
0x2ED: CALL display_on       ; Turn display back on
0x2EF: MOV A,R1              ; Get keystroke result
0x2F0: JMP end_of_select_game  ; Jump to cartridge vector at 0x408
```

## Key Points

1. **Called Once**: The routine is called once by the cartridge at startup
2. **Blocks on Input**: It waits in `get_keystroke` (0x13D) until a key is pressed
3. **get_keystroke Behavior**: 
   - Calls `wait_for_interrupt` (0x176) in a loop
   - Calls `keyboard_routine` (0x0B0) to check for keys
   - Returns only when a valid key is detected (bit 7 clear in R7)
4. **Returns to Cartridge**: Jumps to 0x408 with the selected game number in A

## Debugging Strategy

To debug select_game:

```bash
./debug_select_game.sh
```

This sets breakpoints at:
- **0x2C3** - Entry to select_game
- **0x13D** - Entry to get_keystroke (where it waits)
- **0x0B0** - Keyboard routine (called repeatedly)

### Debugging Commands

When stopped at a breakpoint:
- `dis` - See disassembly around current PC
- `r` - Show CPU registers
- `s` - Step one instruction
- `s 10` - Step 10 instructions
- `c` - Continue to next breakpoint
- `v` - Show VDC registers (to see what's displayed)
- `m 0x2F2 0x2FD` - View the "QUEL JEU?" text data

### Expected Flow

1. Break at 0x2C3 (select_game entry)
2. Step through initialization
3. Step through text display loop (11 iterations)
4. Break at 0x13D (get_keystroke)
5. Inside get_keystroke, will repeatedly break at 0x0B0 (keyboard_routine)
6. Press a key (0-9) on the keyboard
7. get_keystroke returns with key value in A
8. Cleanup and jump to cartridge

## Common Issues

- **No display**: Check VDC registers with `v` command
- **Keys not detected**: Check keyboard routine is being called (break at 0x0B0)
- **Wrong text**: Check memory at 0x2F2 with `m 0x2F2 0x2FD`
