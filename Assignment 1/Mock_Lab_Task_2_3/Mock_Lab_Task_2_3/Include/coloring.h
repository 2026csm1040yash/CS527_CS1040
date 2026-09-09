#ifndef COLORING_H
#define COLORING_H
#include "csr.h"
int *greedy_welsh_powell(const CSRGraph*g,int*colors_used);
int validate_coloring(const CSRGraph*g,const int*c);
#endif
