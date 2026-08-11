#ifndef MEMORY_H
#define MEMORY_H

/* Instruction Memory & Data memory and to make sure it is not included again and again*/
extern unsigned char inst[256];

/* Data Memory */
extern unsigned char Data[256];

/* Memory Functions */
void initialize(const char *program_file, const char *data_file);
void finalize(const char *filename);
#endif