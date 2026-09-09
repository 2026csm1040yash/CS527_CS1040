#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "csr.h"
#include "coloring.h"
#include "pagerank.h"

static void free_adj(int**a,int n){for(int i=0;i<n;i++)free(a[i]);free(a);}
static double timer_now(void){
#ifdef _WIN32
 static LARGE_INTEGER f;LARGE_INTEGER c;if(!f.QuadPart)QueryPerformanceFrequency(&f);QueryPerformanceCounter(&c);return(double)c.QuadPart/f.QuadPart;
#else
 struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;
#endif
}
static int load(const char*fn,int*v,int*e,int***a,int**deg){
 FILE*f=fopen(fn,"r");if(!f)return 0;if(fscanf(f,"%d%d",v,e)!=2||*v<=0||*e<0){fclose(f);return 0;}
 *a=calloc(*v,sizeof(int*));*deg=calloc(*v,sizeof(int));if(!*a||!*deg){fclose(f);return 0;}
 for(int i=0;i<*v;i++){int u,d;if(fscanf(f,"%d%d",&u,&d)!=2||u<0||u>=*v||d<0){fclose(f);return 0;}(*deg)[u]=d;
  if(d){(*a)[u]=malloc(d*sizeof(int));for(int j=0;j<d;j++)if(fscanf(f,"%d",&(*a)[u][j])!=1||(*a)[u][j]<0||(*a)[u][j]>=*v){fclose(f);return 0;}}
 }fclose(f);return 1;
}
static int coloring(const char*fn){
 int v,e,**a,*deg;if(!load(fn,&v,&e,&a,&deg))return 1;CSRGraph*g=create_csr(v,2*e,a,deg);if(!g)return 1;
 double s=timer_now();int nc,*c=greedy_welsh_powell(g,&nc);double z=timer_now();
 printf("Algorithm: Greedy Vertex Coloring\nVertex colors:\n");for(int i=0;i<v;i++)printf("%d %d\n",i,c[i]);
 printf("Colors used: %d\nValid coloring: %s\nExecution time: %.6f ms\n",nc,validate_coloring(g,c)?"true":"false",(z-s)*1000);
 free(c);free_csr(g);free_adj(a,v);free(deg);return 0;
}
static int pr(const char*fn){
 FILE*f=fopen(fn,"r");if(!f)return 1;int v,e;if(fscanf(f,"%d%d",&v,&e)!=2){fclose(f);return 1;}
 int**a=calloc(v,sizeof(int*));int*deg=calloc(v,sizeof(int));for(int i=0;i<v;i++){int u,d;fscanf(f,"%d%d",&u,&d);deg[u]=d;if(d)a[u]=malloc(d*sizeof(int));for(int j=0;j<d;j++)fscanf(f,"%d",&a[u][j]);}
 char key[32];double d,tol;int max;fscanf(f,"%31s%lf",key,&d);fscanf(f,"%31s%lf",key,&tol);fscanf(f,"%31s%d",key,&max);fclose(f);
 CSRGraph*g=create_csr(v,e,a,deg);double*r=malloc(v*sizeof(double));int it,cv;double s=timer_now();pagerank(g,d,tol,max,r,&it,&cv);double z=timer_now();
 double sum=0;int top=0;for(int i=0;i<v;i++){sum+=r[i];if(r[i]>r[top])top=i;}printf("Algorithm: PageRank\nVertex ranks:\n");for(int i=0;i<v;i++)printf("%d %.10f\n",i,r[i]);
 printf("Top vertex: %d\nSum of ranks: %.10f\nIterations: %d\nConverged: %s\nExecution time: %.6f ms\n",top,sum,it,cv?"true":"false",(z-s)*1000);
 free(r);free_csr(g);free_adj(a,v);free(deg);return 0;
}
int main(int argc,char**argv){if(argc!=3){printf("Usage: %s coloring|pagerank input.txt\n",argv[0]);return 1;}return strcmp(argv[1],"coloring")==0?coloring(argv[2]):pr(argv[2]);}
