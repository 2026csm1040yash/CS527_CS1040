#ifndef CSR_H
#define CSR_H
typedef struct{int v,e,*row_ptr,*col_idx;} CSRGraph;
CSRGraph *create_csr(int v,int e,int **adj,int *degree);
void free_csr(CSRGraph *g);
#endif
