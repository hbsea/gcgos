#include "defs.h"

// in kernelvec.S, calls kerneltrap().
void kernel_vec();
//
// set up trapframe and control registers for a return to user space
//
void prepare_return(void)
{
    // EL1 to EL0.
}

void trapinit(void){}

void trapinithart(void)
{
    printf("kernel_vec: %p\n", &kernel_vec);
    w_vbar_el1((uint64)kernel_vec);
    asm volatile("msr daifclr, #0xf");
    printf("vbar_el1: %p\n", r_vbar_el1());
}

void kerneltrap(void)
{
    // printf("r_CurrentEL: %p\n", r_CurrentEL());
    // printf("r_esr_el1: %p\n", r_esr_el1());
    printf("\n");
}