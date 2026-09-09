#include <stdlib.h> 
#include <math.h> 
#include "../Include/pagerank.h" 

int pagerank(const CSRGraph*g,double d,double tol,int max,double*rank,int*iters,int*conv){ 
    int n=g->v;
    if(n<=0||d<0||d>1||tol<=0||max<=0)return 0; 

    double*old=malloc(n*sizeof(double)),*nw=malloc(n*sizeof(double));
    if(!old||!nw){
        free(old);
        free(nw);
        return 0;
    } 

    for(int i=0;i<n;i++)
        old[i]=1.0/n;
    *conv=0;
    *iters=0; 

    for(int it=1;it<=max;it++){
        double dang=0; 

        for(int u=0;u<n;u++)
            if(g->row_ptr[u]==g->row_ptr[u+1])
                dang+=old[u]; 

        double base=(1-d)/n+dang*d/n;
        for(int v=0;v<n;v++)
            nw[v]=base; 

        for(int u=0;u<n;u++){
            int s=g->row_ptr[u],e=g->row_ptr[u+1],deg=e-s;
            if(!deg)continue; 

            double x=d*old[u]/deg;
            for(int p=s;p<e;p++)
                nw[g->col_idx[p]]+=x; 
        } 

        double diff=0;
        for(int i=0;i<n;i++)
            diff+=fabs(nw[i]-old[i]);

        for(int i=0;i<n;i++)
            old[i]=nw[i]; 

        *iters=it;
        if(diff<=tol){
            *conv=1;
            break;
        } 
    } 

    for(int i=0;i<n;i++)
        rank[i]=old[i];

    free(old);
    free(nw);
    return 1; 
}