#include <stdlib.h> 
#include "../Include/csr.h" 

CSRGraph *create_csr(int v,int e,int **adj,int *degree){ 
    CSRGraph*g=malloc(sizeof(*g)); 
    if(!g)return NULL; 
    g->v=v;
    g->e=e; 
    g->row_ptr=malloc((v+1)*sizeof(int));
    g->col_idx=malloc((e?e:1)*sizeof(int)); 

    if(!g->row_ptr||!g->col_idx){
        free(g->row_ptr);
        free(g->col_idx);
        free(g);
        return NULL;
    } 

    g->row_ptr[0]=0; 
    for(int i=0;i<v;i++)
        g->row_ptr[i+1]=g->row_ptr[i]+degree[i]; 

    for(int i=0;i<v;i++)
        for(int j=0;j<degree[i];j++)
            g->col_idx[g->row_ptr[i]+j]=adj[i][j]; 

    return g; 
} 

void free_csr(CSRGraph*g){
    if(g){
        free(g->row_ptr);
        free(g->col_idx);
        free(g);
    }
}