#pragma once
#include "types.h"
#include "arm.h"

// console.c
void consoleinit(void);
void consputc(int);

// printf.c
int printf(char *, ...) __attribute__((format(printf, 1, 2)));
void panic(char *);

// kalloc.c
void kinit(void);
void kfree(void *);
void *kalloc(void);
void countkmem(void);

// vm.c
void kvminit(void);
void kvminithart(void);
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);
int mappages(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 size, uint64 perm);
pagetable_t uvmcreat(void);

// mmu.S
extern void enable_mmu(uint64 ttbr0_el1, uint64 ttbr1_el1, uint64 tcr_el1, uint64 mair_el1);

// proc.c
void proc_mapstacks(pagetable_t);
void procinit(void);
void userinit(void);
void swtch();

// user_proc.c
void uproc1(void);

// kerenlvec.S
void kernelvec(void);

//trap.c
void trapinit(void);
void trapinithart(void);
void prepare_return(void);
void show_invalid_entry_message(int,uint64,uint64) __attribute__((section("trap_msg")));