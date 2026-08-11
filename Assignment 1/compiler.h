#ifndef COMPILER_H
#define COMPILER_H

/* Function to convert source program into bytecode and to make sure it is not included again and again*/
void compile(const char *sourceFile, const char *bytecodeFile);

#endif