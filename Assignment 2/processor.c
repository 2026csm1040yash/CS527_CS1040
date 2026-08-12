#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "processor.h"
#include "memory.h"

int32_t Register[256];
int PC;
int opcode;
int dest;
int src1;
int src2;
int end_of_simulation;
int N, Z, C, V;

static uint32_t read32(int address)
{
    if (address < 0 || address + 3 >= DATA_MEMORY_SIZE)
        return 0;

    return ((uint32_t)Data[address]) |
           ((uint32_t)Data[address + 1] << 8) |
           ((uint32_t)Data[address + 2] << 16) |
           ((uint32_t)Data[address + 3] << 24);
}

static void write32(int address, uint32_t value)
{
    if (address < 0 || address + 3 >= DATA_MEMORY_SIZE)
        return;

    Data[address]     = (unsigned char)(value & 0xFF);
    Data[address + 1] = (unsigned char)((value >> 8) & 0xFF);
    Data[address + 2] = (unsigned char)((value >> 16) & 0xFF);
    Data[address + 3] = (unsigned char)((value >> 24) & 0xFF);
}

static void update_add_flags(int32_t a, int32_t b, int32_t result)
{
    uint32_t ua = (uint32_t)a;
    uint32_t ub = (uint32_t)b;
    uint32_t ur = (uint32_t)result;

    Z = (ur == 0);
    N = ((ur >> 31) & 1U);
    C = (ur < ua || ur < ub);
    V = (((a >= 0) && (b >= 0) && (result < 0)) ||
         ((a < 0) && (b < 0) && (result >= 0)));
}

static void update_sub_flags(int32_t a, int32_t b, int32_t result)
{
    Z = ((uint32_t)result == 0);
    N = (((uint32_t)result >> 31) & 1U);
    C = (a > b);
    V = ((a < 0 && b >= 0 && result >= 0) ||
         (a >= 0 && b < 0 && result < 0));
}

void reset(void)
{
    int i;
    for (i = 0; i < 256; ++i)
        Register[i] = 0;

    PC = 0;
    opcode = dest = src1 = src2 = 0;
    end_of_simulation = 0;
    N = Z = C = V = 0;
}

void fetch(void)
{
    if (PC < 0 || PC + 3 >= INSTRUCTION_MEMORY_SIZE) {
        end_of_simulation = 1;
        return;
    }

    opcode = Instruction[PC];
    dest   = Instruction[PC + 1];
    src1   = Instruction[PC + 2];
    src2   = Instruction[PC + 3];
    PC += 4;
}

void decode(void)
{
    /* Decode is intentionally empty for this lab. */
}

static void execute_branch(int condition, int offset)
{
    int take = 0;

    switch (condition) {
        case 0x0: take = Z; break;                 /* EQ */
        case 0x1: take = !Z; break;                /* NE */
        case 0x2: take = C; break;                 /* CS */
        case 0x3: take = !C; break;                /* CC */
        case 0x4: take = N; break;                 /* MI */
        case 0x5: take = !N; break;                /* PL */
        case 0x6: take = V; break;                 /* VS */
        case 0x7: take = !V; break;                /* VC */
        case 0x8: take = C && !Z; break;            /* HI */
        case 0x9: take = !C || Z; break;            /* LS */
        case 0xA: take = (N == V); break;           /* GE */
        case 0xB: take = (N != V); break;           /* LT */
        case 0xC: take = !Z && (N == V); break;    /* GT */
        case 0xD: take = Z || (N != V); break;     /* LE */
        case 0xE: take = 1; break;                 /* AL / BAL */
        default: break;
    }

    if (take)
        /* The lab defines the branch offset relative to the current
           instruction address. fetch() has already advanced PC to the
           next instruction, so compensate by one instruction. */
        PC += (offset - 1) * 4;
}

void execute(void)
{
    int32_t a, b, result;
    int address;

    switch (opcode) {
        case 0x00:
            end_of_simulation = 1;
            break;

        /* Register-register arithmetic */
        case 0x01:
            a = Register[src1]; b = Register[src2];
            result = a + b;
            Register[dest] = result;
            update_add_flags(a, b, result);
            break;

        case 0x02:
            a = Register[src1]; b = Register[src2];
            result = a - b;
            Register[dest] = result;
            update_sub_flags(a, b, result);
            break;

        case 0x03:
            Register[dest] = Register[src1] * Register[src2];
            break;

        case 0x04:
            if (Register[src2] != 0)
                Register[dest] = Register[src1] / Register[src2];
            break;

        /* Register-constant arithmetic */
        case 0x09:
            a = Register[src1]; b = src2;
            result = a + b;
            Register[dest] = result;
            update_add_flags(a, b, result);
            break;

        case 0x0A:
            a = Register[src1]; b = src2;
            result = a - b;
            Register[dest] = result;
            update_sub_flags(a, b, result);
            break;

        case 0x0B:
            Register[dest] = Register[src1] * src2;
            break;

        case 0x0C:
            if (src2 != 0)
                Register[dest] = Register[src1] / src2;
            break;

        /* Memory read: operand 1 must be zero. */
        case 0x05:
            address = Register[src2];
            Register[dest] = (int32_t)read32(address);
            break;

        /* Memory write: dest is address register, operand 1 is zero,
           operand 2 is value register. */
        case 0x06:
            address = Register[dest];
            write32(address, (uint32_t)Register[src2]);
            break;

        /* Data movement: dest = constant.
           Constant data movement is opcode 0x0F (displayed as F). */
        case 0x0F:
            Register[dest] = src2;
            break;

        /* Memory read using constant address. */
        case 0x0D:
            Register[dest] = (int32_t)read32(src2);
            break;

        /* Memory write using constant address. */
        case 0x0E:
            write32(src1, (uint32_t)Register[src2]);
            break;

        /* Branch: 0x10 + condition code. */
        default:
            if (opcode >= 0x10 && opcode <= 0x1E) {
                execute_branch(opcode - 0x10, (int8_t)src2);
            } else {
                printf("Invalid Opcode : %02X\n", opcode);
                end_of_simulation = 1;
            }
            break;
    }
}
