# Reference Alignment Notes

This version is aligned to the supplied CS527 Lab 2 specification.

## Important corrections

1. `Instruction` memory is 256 bytes.
2. `Data` memory is 4096 bytes.
3. Registers are 32-bit integers.
4. `program.byte` and `data.byte` are hexadecimal byte files.
5. Memory values are 32-bit and occupy four bytes.
6. Memory-read constant-address opcode is `0x0D`.
7. Memory-write variable-address form is `0x06`.
8. Memory-write constant-address form is `0x0E`.
9. Branch bytecode uses `0x10 + condition_code` and an 8-bit relative offset.
10. The specification's branch offset is relative to the current instruction address. Since `fetch()` advances PC by 4 bytes, the processor applies `(offset - 1) * 4` when a branch is taken.
11. The supplied determinant test had an incorrect intermediate calculation. It has been corrected; the determinant of

       1 2 3
       0 4 5
       1 0 6

    is 22.

## Data-movement ambiguity in the supplied specification

The supplied screenshot of the worked example is being used as the reference for the bytecode discrepancy. It shows constant data movement such as `x1 = 0` as `F 1 0 0`, i.e. opcode `0x0F`. The extracted PDF text shows `7 1 0 0`, which conflicts with the screenshot and with the opcode table.

Therefore this version follows the supplied screenshot for the worked example: constant-to-register movement uses opcode `0x0F` and is displayed as `F`. It does not invent register-to-register movement semantics for `x2 = x1`.
