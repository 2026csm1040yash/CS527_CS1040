#ifndef PAGERANK_H
#define PAGERANK_H
#include "csr.h"
int pagerank(const CSRGraph*g,double d,double tol,int max,double*rank,int*iters,int*conv);
#endif
