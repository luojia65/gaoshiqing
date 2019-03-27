#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define T 3

typedef struct Node {
	int n;
	int key[2*T];
	int leaf;
	struct Node *c[2*T+1];
} Node;

Node *b_tree_make_node() {
	Node *x = (Node *)malloc(sizeof(Node));
	x->leaf = 1;
	x->n = 0;
	return x;
}

void b_tree_search(Node *x, int elem, Node **ans, int *ind) {
	int i=1;
	while(i<x->n && elem>x->key[i]) i++;
	if(i<x->n && elem==x->key[i]) { *ans=x, *ind=i; return; }
	else if(x->leaf==1) { *ans=NULL; return; }
	else b_tree_search(x->c[i], elem, ans, ind);
}

void b_tree_split_child(Node *x, int i) {
	Node *z = b_tree_make_node();
	Node *y = x->c[i];
	z->leaf = y->leaf;
	z->n = T-1; 
	for(int j=1;j<=T-1;++j) 
		z->key[j]=y->key[j+T]; 
	if(y->leaf==0) 
		for(int j=1;j<=T;++j) z->c[j]=y->c[j+T]; 
	y->n = T-1;
	for(int j=x->n+1;j>=i+1;j--) 
		x->c[j+1]=x->c[j];
	x->c[i+1]=z;
	for(int j=x->n;j>=i;--j) 
		x->key[j+1]=x->key[j];
	x->key[i]=y->key[T];
	x->n++;
}

void b_tree_insert_nonfull(Node *x, int elem) {
	int i=x->n;
	if(x->leaf) {
		while(i>=1 && elem<x->key[i]) x->key[i+1]=x->key[i],i--;
		x->key[i+1]=elem;
		x->n++;
	} else { 
		while(i>=1 && elem<x->key[i]) i--;
		i++;
		if(x->c[i]->n == 2*T-1) {
			b_tree_split_child(x, i);
			if(elem > x->key[i]) i++;
		}
		b_tree_insert_nonfull(x->c[i], elem); 
	}
}

void b_tree_insert(Node **root, int elem) {
	Node *r = *root;
	if((*root)->n == 2*T-1) {
		Node *s = b_tree_make_node();
		*root = s;
		s->leaf = 0;
		s->n = 0;
		s->c[1] = r;
		b_tree_split_child(s, 1);
		b_tree_insert_nonfull(s, elem);
	}	
	else b_tree_insert_nonfull(r, elem);
}

void b_tree_remove(Node *x, int elem) {
	int i=1;
	while(i<=x->n && x->key[i]<elem) i++;
	if(i<=x->n && elem==x->key[i]) { 
		if(x->leaf) {
			// drop_in_place(x.key[i])
			while(i<=x->n-1) x->key[i]=x->key[i+1], ++i;
			x->n--;
		} else {
			Node *y=x->c[i];
			Node *z=x->c[i+1];
			if(y->n>=T) {
				int k1=y->key[y->n];
				x->key[i]=k1; 
				b_tree_remove(y,k1);
			} else if(z->n>=T) {
				int k1=z->key[1];
				x->key[i]=k1; 
				b_tree_remove(z,k1);
			} else { 
				y->key[T]=elem;
				for(int j=1;j<=T-1;++j) y->key[T+j]=z->key[j];
				if(y->leaf==0) 
					for(int j=1;j<=T-1;++j) y->c[T+j]=z->c[j];
				free(z);
				for(int j=i;j<=x->n-1;++j) {
					x->key[i]=x->key[i+1];
					x->c[i+1]=x->c[i+2]; 
				}
				x->n--;
				y->n=2*T-1; 
				b_tree_remove(y,elem);
			}
		}
	} else {
		Node *m=x->c[i];
		if(m->n==T-1) {
			if(i>=2&&x->c[i-1]->n>=T) {
				Node *l=x->c[i-1];
				for(int i=T-1;i>=1;--i) m->key[i+1]=m->key[i];
				m->key[1]=x->key[i-1];
				x->key[i-1]=l->key[l->n];
				if(m->leaf==0) {
					for(int i=T;i>=1;--i) m->c[i+1]=m->c[i];
					m->c[1]=l->c[l->n+1];
				}
				l->n--;
				m->n++;
				b_tree_remove(m,elem);
			} else if (i<=x->n&&x->c[i+1]->n>=T) {
				Node *r=x->c[i+1];
				m->key[T]=x->key[i];
				x->key[i]=r->key[1];
				for(int i=1;i<=r->n-1;++i) r->key[i]=r->key[i+1];
				if(m->leaf==0) {
					m->c[T+1]=r->c[1];
					for(int i=1;i<=r->n;++i) r->c[i]=r->c[i+1];
				}
				r->n--;
				m->n++;
				b_tree_remove(m,elem);
			} else if (i>=2){
				Node *l=x->c[i-1];
				for(int j=1;j<=m->n;++j) 
					l->key[j+T]=m->key[j];
				l->key[T]=x->key[i];
				if(l->leaf==0) for(int j=1;j<=m->n+1;++j) 
					l->c[j+T]=m->c[j];
				for(int j=i;j<=x->n-1;++j) {
					x->key[j]=x->key[j+1];
					x->c[j]=x->c[j+1];
				}
				x->n--;
				l->n=2*T-1;
				free(m);
				b_tree_remove(l,elem); 
			} else if (i==1) {
				Node *r=x->c[i+1];
				for(int j=1;j<=r->n;++j)
					m->key[j+T]=r->key[j];
				m->key[T]=x->key[i];
				if(r->leaf==0) for(int j=1;j<=r->n+1;++j) 
					m->c[j+T]=r->c[j];
				for(int j=i;j<=x->n-1;++j) {
					x->key[j]=x->key[j+1];
					x->c[j+1]=x->c[j+2];
				}
				x->n--;
				m->n=2*T-1;
				free(r);
				b_tree_remove(m,elem);
			}
		}
	} 
} 

Node *tree;

void main() {
	tree = b_tree_make_node();
	for(int i=12;i>=1;i-=1) {
		b_tree_insert(&tree, i);
	}
	for(int i=21;i>=13;i-=1) {
		b_tree_insert(&tree, i);
	}
//	for(int i=1;i<=18;++i) {
//		Node *ptr; int ind;
//		b_tree_search(tree, i, &ptr, &ind);
//		printf("%d => %d\n", i, ind);
//	}
	for(int i=1;i<=21;i++) 
		b_tree_remove(tree, i); 
}
 
