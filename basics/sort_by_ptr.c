#include <stdio.h>

void swap(int *a, int *b) {
    int t=*a;
    *a=*b;
    *b=t;
}

int *qs_partition(int *f, int *t) {
    int x=*t;
    int *i=f;
    for(int *j=f;j<=t;++j) {
        if(*j<x) {
            swap(i, j);
            i+=1;
        }
    }
    swap(i, t);
    return i;
}

void quicksort(int *f, int *t) {
    if(f<t) {
        int *mi=qs_partition(f, t);
        quicksort(f,mi-1);
        quicksort(mi+1,t);
    }
}

void main() {
    int A[] = {3,5,2,1,4};
    quicksort(A,A+5-1);
    for(int i=0;i<5;++i) printf("%d,",A[i]);
    putchar('\n');
}
