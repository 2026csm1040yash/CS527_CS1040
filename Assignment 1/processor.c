#include <stdio.h>

#include "processor.h"
#include "memory.h"

/* Processor Registers */
int Register[256];

/* Processor Variables */
int PC;
int opcode;
int dest;
int src1;
int src2;

int end_of_simulation = 0;

/* Reset Processor */
void reset()
{
    int i;

    for(i = 0; i < 256; i++)
    {
        Register[i] = 0;
    }

    PC = 0;//we have to this here too because after execution it will contain some different value
    end_of_simulation = 0;//we have to this here too because after execution it will contain some different value
}

/* Fetch Next Instruction */
void fetch()
{
    opcode = inst[PC];
    dest   = inst[PC + 1];
    src1   = inst[PC + 2];
    src2   = inst[PC + 3];

    PC = PC + 4;
}

// /* Decode (Empty for Lab 1) */
// void decode()
// {

// }

/* Execute Instruction */
void execute()
{
    switch(opcode)
    {
        case 0:
            end_of_simulation = 1;
            break;

        case 1:     /* ADD */
            Register[dest] = Register[src1] + Register[src2];
            break;

        case 2:     /* SUBTRACT */
            Register[dest] = Register[src1] - Register[src2];
            break;

        case 3:     /* MULTIPLY */
            Register[dest] = Register[src1] * Register[src2];
            break;

        case 4:     /* DIVIDE */
            if(Register[src2] != 0)
            {
                Register[dest] = Register[src1] / Register[src2];
            }
            break;

        case 5:     /* READ */
            Register[dest] = Data[src1];
            break;

        case 6:     /* WRITE */
            Data[src1] = Register[dest];
            break;

        case 7:     /* DATA MOVEMENT */
            Register[dest] = src1;
            break;

        default:
            printf("Invalid Opcode : %d\n", opcode); //by any mistake invalid opcode is generated
            end_of_simulation = 1;
            break;
    }
}