#include <stdio.h>

void insert_sort(int *A, int n) {
    for(int i=1;i<n;++i) {
        int tmp=A[i];
        int j=i-1;
        while(j>=0 && A[j]>tmp) {
            A[j+1]=A[j];
            j-=1;
        }
        if(j!=i-1) A[j+1]=tmp;
    }
}

void swap(int *a, int *b) {
    int t=*a;
    *a=*b;
    *b=t;
}

void bubble_sort(int *A, int n) {
    for(int i=0;i<n;++i) 
        for(int j=i;j<n;++j) 
            if(A[j]<A[i]) 
                swap(&A[i], &A[j]);
}

#define LC(x) (2*x+1)
#define RC(x) (2*x+2)
#define PAR(x) ((x-1)/2)

void max_heapify(int *A, int n, int i) {
    int l=LC(i);
    int r=RC(i);
    int mx=i;
    if(l<n && A[l]>A[i]) mx=l;
    if(r<n && A[r]>A[mx]) mx=r;
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

int qs_partition(int *A, int f, int t) {
    int x=A[t];
    int i=f;
    for(int j=f;j<=t;++j) {
        if(A[j]<x) {
            swap(&A[i],&A[j]);
            i+=1;
        }
    }
    swap(&A[i],&A[t]);
    return i;
}

void do_quicksort(int *A, int f, int t) {
    if(f<t) {
        int mi=qs_partition(A,f,t);
        do_quicksort(A,f,mi-1);
        do_quicksort(A,mi+1,t);
    }
}

void quicksort(int *A,int n) {
    do_quicksort(A,0,n-1);
}

void main() {
    int A[]={5,3,1,4,2,6,7,9,0,8};
    heap_sort(A,10);
    for(int i=0;i<10;++i) printf("%d,",A[i]);
    putchar('\n');
}
