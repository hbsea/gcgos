#include "defs.h"
#include "trap.h"
#include "vm.h"
// in kernelvec.S, calls kerneltrap().
void vectors();
//
// set up trapframe and control registers for a return to user space
// EL1->EL0.
// before eret it's need set spsr_el1:el0h ttbr0_el1:pagetable_mmu sp_el0:user_stack_point elr_el1:user_text
void prepare_return(void)
{
	w_spsr_el1(0b0000 | (1 << 6) | (1 << 7) | (1 << 8));
	asm volatile("eret");
}

void trapinit(void) {}

void trapinithart(void)
{
	w_ttbr0_el1((uint64)kernel_pagetable);
	printf("vectors: %p\n", &vectors);
	w_vbar_el1((uint64)vectors);
	asm volatile("msr daifclr, #0xF");
	printf("vbar_el1: %p\n", r_vbar_el1());
}

// EL1->EL1
void kerneltrap(void)
{
	printf("kerneltrap");
	// printf("r_esr_el1: %b\n", ((r_esr_el1())>>26 & 0x3F));
}

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",
	"FIQ_INVALID_EL1t",
	"ERROR_INVALID_EL1T",

	"SYNC_INVALID_EL1h",
	"IRQ_INVALID_EL1h",
	"FIQ_INVALID_EL1h",
	"ERROR_INVALID_EL1h",

	"SYNC_INVALID_EL0_64",
	"IRQ_INVALID_EL0_64",
	"FIQ_INVALID_EL0_64",
	"ERROR_INVALID_EL0_64",

	"SYNC_INVALID_EL0_32",
	"IRQ_INVALID_EL0_32",
	"FIQ_INVALID_EL0_32",
	"ERROR_INVALID_EL0_32"};

void show_invalid_entry_message(int type, uint64 esr, uint64 address)
{
	printf("%s, ESR[EC]: %b, address: %x\r\n", entry_error_messages[type], ((esr >> 26) & 0x3f), address);
	panic("trap message");
}