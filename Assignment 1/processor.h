#ifndef PROCESSOR_H
#define PROCESSOR_H

//contain global variables
/* Processor Registers */
extern int Register[256];

/* Processor Variables */
extern int PC;
extern int opcode;
extern int dest;
extern int src1;
extern int src2;

/* Simulation Flag  ie to indicate whether while loop should run or not*/
extern int end_of_simulation;

/* Processor Functions and to make sure function are not included again and again*/
void reset(void);
void fetch(void);
void decode(void);
void execute(void);

#endif