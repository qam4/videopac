Reading 8048 Series Code
Here are a few notes and (hopefully) helpful pointers for anyone looking to make sense of Intel MCS48 series code. All this info and more can be found in the Intel MCS48 User’s Manual and the MCS48 Assembly Language Reference Manual.

If you have any experience with 8051 microcontrollers, the 8048 should be fairly familiar. However, it’s not an 8051; it has fewer resources, and a simpler instruction set. This tends to result in more complicated code for a given task.

Resets and Interrupts
Program execution starts at location 0 as you’d expect. The 8048 has 2 interrupt sources: the external interrupt pin and the timer. Here are the 3 most important addresses to memorize:

Address	Function
0x000	start/reset
0x003	external interrupt
0x007	timer
The timer counts up and triggers an interrupt when it counts up from 255.

The external interrupt is triggered on the falling edge of the INT pin.

The instruction retr signals the end of an interrupt routine; this causes the program coutner to be restored from the stack.

A reset (triggered by pulsing the RESET pin) sets the program counter to 0. The reset feature doesn’t erase any RAM or special registers. It does disable interrupts, and it sets all port pins to high-impeadence output. It can be used as a pseudo-interrupt - you just have to make sure to set up all the port pin outputs and interrupt configuration bits in the reset routine.

A complete list of what the reset does can be found in the manual (linked above).

Special Registers and Register Banks
The 8048 has a set of general purpose registers known as r0 - r7. These registers can be used for several important tasks that you can’t do on general RAM locations directly.

Of these r0 and r1 are special: they can be used to load values from RAM indirectly. For example:

mov  r0,#$24
mov  a,@r0
This code loads the value 0x24 into r0, and then loads the value from RAM location 0x24 into the accumulator. The register r1 can be used in the same way, but the others (rb2 to rb7) cannot.

All 8 of these registers (including the special r0 and r1) can also be used for arithmetic with the value in the accumulator, and can also be incremented, and decremented directly. They can also be used for the all-important djnz instruction, which makes them useful as counters:

0x1a djnz r2,$0036    ;dec. r2, and jump to 0x36 if the result isn't zero
0x1d mov  r0,#$26
0x1e mov  a,@r0
...                   ;(more instructions, not shown)
...
0x36 jmp  $005E       ;do something else if the r2 did reach zero
There are actually two completely separate sets of registers, known as register banks. The progam must select which register bank is to be used.

By default, register bank 0 is selected. The register bank can be explicitly selected using the sel instruction:

sel  rb0
...
sel  rb1
The idea behind this is that one bank can be used for the main program, and the other can be used for interrupt routines. One of the first instructions in an interrupt routine should be to select the alternate register bank. This way, the interrupt routine has it’s own dedicated registers for loading variables from RAM, it’s own counters etc. When an interrupt calls retr, the previously selected register bank is restored automatically.

The register banks are memory-mapped; this means that they can be accessed directly instead of through the special names rx - but that results in a program that’s hard to understand and maintain, so it’s probably not a good idea unless there’s a compelling reason to do it.

Memory Banks
The 8048 variations have different amounts of program memory; this section is about the 4K version. I’m not sure about the 2K versions.

In the 4K version, program memory is divied into 2 banks of 2K each. Furthermore, each bank is divided into pages of 256 instructions. These boundaries matter when using any instruction that makes the program counter jump - that is, call, movp and the various jump instructions.

In the following subsections we’ll look at how each type of instruction is affected by these memory boundaries.

Call and Unconditional Jump
The call and unconditional jump instructions are limited to the current bank. The current bank is determined by the sel instruction:

sel mb0    ;select memory bank 0
...
sel mb1    ;select memory bank 1
The confusing thing about this is that the addresses specified by call and jmp instructions are relative to the current bank. So, for instance, you might see something like this:

call $0608
…and when you look at program memory location 0x608, it makes no sense:

0x608 nop
That’s because the call instruction is in a part of the code that uses mb1, so the location 0x608 is interpreted relative to the start of mb1. Since the banks are 2k, the first location of mb1 is 0x800 (2048 in decimal). When you add 0x608 to 0x800 you get 0xE08, and that is the true location of the subroutine that this call instruction references.

Sometimes you’ll see a subroutine call being bracketed within sel instructions like this:

0xf0e sel  mb0
0xf10 call $0300
0xf11 sel  mb1
This is necessary when we are in mb1 but we want to call a subroutine that’s located in mb0. We switch to bank 0, call the subroutine, and then switch back to bank 1 again.

Note: changing program memroy banks does not affect the register banks; switching register banks is done with sel rb0 or sel rb1 and it completely seprate from program memory banks.

Conditional Jumps
Conditional jump instructions (jbx, djnz etc.) use absolute addresses but they’re limited to the current page (256 instructions). This can be a little confusing to think about, but it actually makes code very easy to read. When you see something like this:

jb1 $0A7C
…you can assume 2 things:

the address 0xA7C is the true address that this instruction refereces - you don’t need to translate it based on the currently selected memory bank (as you would for call, or an unconditional jmp).

the location is always nearby; conditional jump instructions are used for the equivalent of if statements from high-level languages, but never to call subroutines or jump to a completely different area of the prgram.

Movp
The movp instruction allows program memory to be read as data. This is how lookup tables (also sometimes known as maps) are implemented.

A movp instruction looks like this:

movp a,@a
Assume that when this instruction is about to run, the accumulator contains a program memory address. This instruction then loads the value stored at that address into the accumulator. It’s up to the programmer to make sure that the address pointed to by the accumulator contains meaningful data. The really important thing to note here is that the address stored in the accumulator before this instruction is executed is relative to the start of the current page.

For example, let’s say the above instruction occurs at progam memory location 0xA72, and let’s assume that the accumulator contains the value 4. The page that location 0xA72 lives in starts at the previous multiple of 256, which is 0xA00 (2560 decimal). So the instruction above will add 4 to 0xA00 to get 0xA04, and then put the value found in 0xA04 into the accumulator.

For this reason, maps or lookup tables are almost always stored at the beginning of a page - this allows them to be read using addresses that start at zero, which makes life easy for the programmers.

Arithmetic
The 8048 has a simple intruction set that doesn’t include multiply or divide instructions. That means we have to get clever to do basic arithmetic, and if you haven’t worked with very simple instruction sets before, it can be hard to grok what’s going on. So in this section we’ll take a look at a few common tricks used in the KLR code.

Modular Arithmetic
You might know that modular arithmetic (aka clock arithemetic) involves dividing a number using integer division, and then discarding the answer (or quotient) and just taking the remainder (or modulus) instead. I won’t get into why this is a useful thing in microcontroller programming - that will speak for itself when you read the code. I just want to show you how it’s actually done.

In general, you can take the modulus of a number with any power of 2 by simply ANDing the number with one less than that power of 2. So for example:

anl  a,#$7
This effectively divides the value in the accumulator by 8 and stores the remainder in the accumulator.