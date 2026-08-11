CS527 Lab 2 / Assignment 2

Build (MinGW/MSYS):
    mingw32-make clean
    mingw32-make

Run:
    .\compiler.exe .\testcase\spec_example.txt .\testcase\data.byte

The compiler prints bytecode to the terminal in hexadecimal, matching the lab example.
Important Lab 2 discrepancies implemented:
- Memory read using a variable: 05 dest 00 address-register
- Memory write using a variable address: 06 address-register 00 value-register
- Legacy Read/Write use the same zero first operand format
- Memory read using constant address uses opcode 0D
- Memory write using constant address uses opcode 0E
- Data movement is 07 dest 00 constant
- Branches are 10 + condition code
