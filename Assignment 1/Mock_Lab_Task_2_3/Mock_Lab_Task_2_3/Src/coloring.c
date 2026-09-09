#include <stdlib.h>
#include "../include/coloring.h"
typedef struct{int v,d;} VD;

static int cmp(const void*a,const void*b)
{
    const VD*x=a,*y=b;
    return x->d!=y->d?y->d-x->d:x->v-y->v;
}
int*greedy_welsh_powell(const CSRGraph*g,int*used_colors)
{
 int n=g->v,*c=malloc(n*sizeof(int)),*used=calloc(n?n:1,sizeof(int));
 VD*o=malloc(n*sizeof(VD));
 if(!c||!used||!o){
    free(c);free(used);free(o);
    return NULL;
}
 for(int i=0;i<n;i++)
 {
    c[i]=-1;
    o[i]=(VD){i,g->row_ptr[i+1]-g->row_ptr[i]};
}
 qsort(o,n,sizeof(VD),cmp);
 int mx=-1;
 for(int k=0;k<n;k++)
 {
  int u=o[k].v;
  for(int i=0;i<n;i++)used[i]=0;
  for(int p=g->row_ptr[u];p<g->row_ptr[u+1];p++)
  {
    int v=g->col_idx[p];if(c[v]>=0)used[c[v]]=1;}
    int col=0;
    while(col<n&&used[col]){
    col++;
    c[u]=col;
    if(col>mx)mx=col;
 }
}
 *used_colors=mx+1;
 free(used);
 free(o);
 return c;
}
int validate_coloring(const CSRGraph*g,const int*c){
 for(int u=0;u<g->v;u++){
    for(int p=g->row_ptr[u];p<g->row_ptr[u+1];p++){
        if(c[u]==c[g->col_idx[p]])return 0;
    }
 }
 return 1;
}
