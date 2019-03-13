#include <stdio.h>
#include <stdlib.h>

#define M 10005
#define D 4
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

typedef struct _d_heap {
    int n;
    int *A;
} d_heap;

void max_heapify(d_heap *heap, int i) {
    int mx=i;
    for(int p=LC(i);p<=min(heap->n-1,RC(i));++p) {
        if(heap->A[p]>heap->A[mx]) mx=p;
    }
    if(mx!=i) {
        swap(&heap->A[i],&heap->A[mx]);
        max_heapify(heap,mx);
    }
}

d_heap *build_max_heap(int *A, int n) {
    d_heap *ans = (d_heap *)malloc(sizeof(d_heap));
    ans->n=n;
    ans->A=A;
    for(int i=PAR(n);i>=0;--i) 
        max_heapify(ans,i);
    return ans;
}

void free_heap(d_heap *heap) {
    free(heap);
}

// unsafe iterator

typedef int *heap_iter_t;

heap_iter_t iter_begin(d_heap *heap) { 
	return &heap->A[0]; // &heap->A[0]
}

heap_iter_t iter_end(d_heap *heap) {
    return &heap->A[0] + heap->n;
}

void main() {
    int A[] = {1,4,5,8,6,3,6,8,9,3,1,3,4};
    d_heap *heap=build_max_heap(A,13);
    for(heap_iter_t iter=iter_begin(heap);iter!=iter_end(heap);iter++) {
        int a = *iter;
        printf("%d, ", a);
    }
    free_heap(heap);
}
