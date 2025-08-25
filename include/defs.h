#include "types.h"

// console.c
void consputc(int);

// printf.c
int printf(char *, ...) __attribute__((format(printf, 1, 2)));
void panic(char *);

// kalloc.c
void kinit(void);
void kfree(void *);
void* kalloc(void);