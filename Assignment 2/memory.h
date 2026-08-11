#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define INSTRUCTION_MEMORY_SIZE 256
#define DATA_MEMORY_SIZE 4096

extern unsigned char Instruction[INSTRUCTION_MEMORY_SIZE];
extern unsigned char Data[DATA_MEMORY_SIZE];

void initialize(const char *program_file, const char *data_file);
void finalize(const char *filename);

#endif
