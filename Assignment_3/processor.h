#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>

#define INTEGER_REGISTERS 256
#define VECTOR_REGISTERS 32
#define VECTOR_LANES 8

extern int32_t Register[INTEGER_REGISTERS];
extern int32_t VectorRegister[VECTOR_REGISTERS][VECTOR_LANES];
extern int PC;
extern int opcode;
extern int dest;
extern int src1;
extern int src2;
extern int end_of_simulation;
extern int N, Z, C, V;

void reset(void);
void fetch(void);
void decode(void);
void execute(void);

#endif
