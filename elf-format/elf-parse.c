#include <stdio.h>
#include <stdlib.h>

#define u8 unsigned char
#define usize unsigned long int

u8 *buf;

void main() {
	FILE *fp = fopen("gpio-interrupt-elf", "rb");
	if(fp == NULL) {
		printf("Error when opening file\n");
		return;	
	}
	fseek(fp, 0, SEEK_END);
	usize len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (u8 *)malloc(len * sizeof(u8));
	fread(buf, len, 1, fp);
	printf("Magic: 0x%x, '%c', '%c', '%c'\n", 
		buf[0], buf[1], buf[2], buf[3]);
	
} 

