#include <stdio.h>
#include <stdint.h>
#include "processor.h"
#include "memory.h"

int32_t Register[INTEGER_REGISTERS];
int32_t VectorRegister[VECTOR_REGISTERS][VECTOR_LANES];
int PC, opcode, dest, src1, src2, end_of_simulation;
int N, Z, C, V;

static uint32_t read32(int address)
{
    if (address < 0 || address + 3 >= DATA_MEMORY_SIZE) return 0;
    return (uint32_t)Data[address] |
           ((uint32_t)Data[address + 1] << 8) |
           ((uint32_t)Data[address + 2] << 16) |
           ((uint32_t)Data[address + 3] << 24);
}

static void write32(int address, uint32_t value)
{
    if (address < 0 || address + 3 >= DATA_MEMORY_SIZE) return;
    Data[address] = (unsigned char)(value & 0xFF);
    Data[address + 1] = (unsigned char)((value >> 8) & 0xFF);
    Data[address + 2] = (unsigned char)((value >> 16) & 0xFF);
    Data[address + 3] = (unsigned char)((value >> 24) & 0xFF);
}

static void update_add_flags(int32_t a, int32_t b, int32_t r)
{
    uint32_t ua=(uint32_t)a, ub=(uint32_t)b, ur=(uint32_t)r;
    Z=(ur==0); N=(int)((ur>>31)&1U); C=(ur<ua || ur<ub);
    V=((a>=0 && b>=0 && r<0) || (a<0 && b<0 && r>=0));
}

static void update_sub_flags(int32_t a, int32_t b, int32_t r)
{
    uint32_t ur=(uint32_t)r;
    Z=(ur==0); N=(int)((ur>>31)&1U); C=(a>b);
    V=((a<0 && b>=0 && r>=0) || (a>=0 && b<0 && r<0));
}

void reset(void)
{
    int i,j;
    for(i=0;i<INTEGER_REGISTERS;i++) Register[i]=0;
    for(i=0;i<VECTOR_REGISTERS;i++) for(j=0;j<VECTOR_LANES;j++) VectorRegister[i][j]=0;
    PC=0; opcode=dest=src1=src2=0; end_of_simulation=0; N=Z=C=V=0;
}

void fetch(void)
{
    if(PC<0 || PC+3>=INSTRUCTION_MEMORY_SIZE){ end_of_simulation=1; return; }
    opcode=Instruction[PC]; dest=Instruction[PC+1]; src1=Instruction[PC+2]; src2=Instruction[PC+3]; PC+=4;
}

void decode(void) { }

static void execute_branch(int condition, int offset)
{
    int take=0;
    switch(condition){
        case 0x0: take=Z; break; case 0x1: take=!Z; break;
        case 0x2: take=C; break; case 0x3: take=!C; break;
        case 0x4: take=N; break; case 0x5: take=!N; break;
        case 0x6: take=V; break; case 0x7: take=!V; break;
        case 0x8: take=C&&!Z; break; case 0x9: take=!C||Z; break;
        case 0xA: take=(N==V); break; case 0xB: take=(N!=V); break;
        case 0xC: take=!Z&&(N==V); break; case 0xD: take=Z||(N!=V); break;
        case 0xE: take=1; break;
    }
    if(take) PC += (offset-1)*4;
}

static void vector_arith(int op, int d, int a, int b, int rhs_constant)
{
    int i;
    for(i=0;i<VECTOR_LANES;i++){
        int32_t rhs = rhs_constant ? b : VectorRegister[b][i];
        if(op==0) VectorRegister[d][i]=VectorRegister[a][i]+rhs;
        else if(op==1) VectorRegister[d][i]=VectorRegister[a][i]-rhs;
        else VectorRegister[d][i]=VectorRegister[a][i]*rhs;
    }
}

static void vector_load(int v, int address)
{
    int i;
    for(i=0;i<VECTOR_LANES;i++) VectorRegister[v][i]=(int32_t)read32(address+i*4);
}

static void vector_store(int v, int address)
{
    int i;
    for(i=0;i<VECTOR_LANES;i++) write32(address+i*4,(uint32_t)VectorRegister[v][i]);
}

void execute(void)
{
    int32_t a,b,r;
    int address;
    switch(opcode){
        case 0x00: end_of_simulation=1; break;
        case 0x01: a=Register[src1]; b=Register[src2]; r=a+b; Register[dest]=r; update_add_flags(a,b,r); break;
        case 0x02: a=Register[src1]; b=Register[src2]; r=a-b; Register[dest]=r; update_sub_flags(a,b,r); break;
        case 0x03: Register[dest]=Register[src1]*Register[src2]; break;
        case 0x04: if(Register[src2]!=0) Register[dest]=Register[src1]/Register[src2]; break;
        case 0x05: address=Register[src2]; Register[dest]=(int32_t)read32(address); break;
        case 0x06: address=Register[dest]; write32(address,(uint32_t)Register[src2]); break;
        case 0x07: Register[dest]=Register[src1]; break;
        case 0x09: a=Register[src1]; b=src2; r=a+b; Register[dest]=r; update_add_flags(a,b,r); break;
        case 0x0A: a=Register[src1]; b=src2; r=a-b; Register[dest]=r; update_sub_flags(a,b,r); break;
        case 0x0B: Register[dest]=Register[src1]*src2; break;
        case 0x0C: if(src2!=0) Register[dest]=Register[src1]/src2; break;
        case 0x0D: Register[dest]=(int32_t)read32(src2); break;
        case 0x0E: write32(src1,(uint32_t)Register[src2]); break;
        case 0x0F: Register[dest]=src2; break;
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
            execute_branch(opcode-0x10,(int8_t)src2); break;
        case 0x21: vector_arith(0,dest,src1,src2,0); break;
        case 0x22: vector_arith(1,dest,src1,src2,0); break;
        case 0x23: vector_arith(2,dest,src1,src2,0); break;
        case 0x25: vector_load(dest,Register[src2]); break;
        case 0x26: vector_store(src2,Register[dest]); break;
        case 0x29: vector_arith(0,dest,src1,src2,1); break;
        case 0x2A: vector_arith(1,dest,src1,src2,1); break;
        case 0x2B: vector_arith(2,dest,src1,src2,1); break;
        case 0x2C: vector_load(dest,src2); break;
        case 0x2E: vector_store(src2,src1); break;
        default: printf("Invalid Opcode : %02X\n",opcode); end_of_simulation=1; break;
    }
}
