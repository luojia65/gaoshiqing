#include <stdio.h>

#define D (4)
#define PAR(x) ((x-1)/D)
#define LC(x) (D*x+1)
#define RC(x) (D*x+D)

void swap(int *a, int *b) {
    int t=*a;
    *a=*b;
    *b=t;
}

int min(int a, int b) {
    return a>b?b:a;
}

void max_heapify(int *A, int n, int i) {
    int mx=i;
    for(int p=LC(i);p<=min(n-1,RC(i));++p) {
        if(A[p]>A[mx]) mx=p;
    }
    if(mx!=i) {
        swap(&A[i], &A[mx]);
        max_heapify(A,n,mx);
    }
}

void build_heap(int *A, int n) {
    for(int i=PAR(n);i>=0;--i) 
        max_heapify(A,n,i);
}

void heap_sort(int *A, int n) {
    build_heap(A,n);
    for(int i=n-1;i>=0;--i) {
        swap(&A[i],&A[0]);
        n-=1;
        max_heapify(A,n,0);
    }
}

void main() {
    int A[] = {3,5,2,1,4,7,9,0,6,8};
    heap_sort(A,10);
    for(int i=0;i<10;++i) printf("%d,",A[i]);
    putchar('\n');
}
