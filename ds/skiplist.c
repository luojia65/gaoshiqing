#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_HEIGHT 5

typedef struct Node {
    int elem;
    int forward_len;
    struct Node *forward[MAX_HEIGHT];
} Node;

int next_height() {
    int ans = 1;
    while (rand() <= RAND_MAX / 2 && ans < MAX_HEIGHT) {
        ans += 1;
    }
    return ans;
}

Node *head = NULL;

void do_insert() {
    int elem;
    scanf("%d",&elem);
    int height = next_height();
    Node *new_node = (Node *)malloc(sizeof(Node));
    memset(new_node, 0, sizeof(Node));
    new_node->forward_len = height;
    new_node->elem = elem;
    int i = MAX_HEIGHT - 1;
    Node *cur = head;
    while(1) {
    	Node *nxt = cur->forward[i];
        if(i != 0 && nxt == NULL) i--;
        else if (i != 0 && nxt != NULL) {
            if (elem < nxt->elem) i--;
            else cur = nxt;
        } else if (i == 0 && nxt != NULL) {
            if (elem < nxt->elem) {	
            	for(int j = MAX_HEIGHT - 1; j >= 0; j--) {
					
				}
                break;
            }
            else cur = cur->forward[i];
        } else if(i == 0 && nxt == NULL) {
        	cur = head;
        	for(int j = height - 1; j >= 0; j--) {
				if (j < cur->forward_len) {
					if (cur->forward[j] == NULL) 
						cur->forward[j] = new_node;
					else cur = cur->forward[j];
				}
			}
            break;   
        }
    }    
}

void main() {
    char cmd;
    head = (Node *)malloc(sizeof(Node));
	memset(head, 0, sizeof(Node)); 
	head->forward_len = MAX_HEIGHT;
    while(~scanf("%c", &cmd)) {
        switch (cmd) {
            case 'i': do_insert(); 
        }
    }   
}
