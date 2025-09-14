#include "defs.h"
#include "trap.h"
#include "vm.h"
#include "memlayout.h"
#include "proc.h"

// in kernelvec.S, calls kerneltrap().
void kernelvec();
//
// set up trapframe and control registers for a return to user space
// EL1->EL0.
// before eret it's need set spsr_el1:el0h ttbr0_el1:pagetable_mmu sp_el0:user_stack_point elr_el1:user_text
// from EL0 to EL1, the hardware automatically sets PSTATE.SP to 1.means use sp_el1
void prepare_return(void)
{

	w_vbar_el1(TRAMPOLINE);
	w_spsr_el1(0b0000 | (1 << 6) | (1 << 7) | (1 << 8));
	asm volatile("eret");
}

void trapinit(void) {}

void trapinithart(void)
{
	w_ttbr0_el1((uint64)kernel_pagetable);
	printf("kerenl_vectors: %p\n", &kernelvec);
	w_vbar_el1((uint64)kernelvec);
	asm volatile("msr daifclr, #0xF");
	printf("vbar_el1: %p\n", r_vbar_el1());
}

// EL1->EL1
void kerneltrap(void)
{

	printf("user trap SPSR_EL1:%p \n", r_spsr_el1());
	int kec = (((r_esr_el1()) >> 26) & 0x3f);
	printf("ec: %b not handle\n", kec);
	panic("GET KERNEL TRAP");

	// printf("r_esr_el1: %b\n", ((r_esr_el1())>>26 & 0x3F));
}
void usertrap(void)
{

	w_vbar_el1((uint64)kernelvec);

	// SPSR_EL1[3:0]异常前的 CPU 模式（EL）
	//  0b0000 = EL0t（用户态）
	//  0b0100 = EL1t（内核态 SP_EL0）
	//  0b0101 = EL1h（内核态 SP_EL1）
	printf("user trap SPSR_EL1:%p \n", r_spsr_el1());
	int ec = (((r_esr_el1()) >> 26) & 0x3f);
	switch (ec)
	{
	case 0b010101:
		syscall();
		break;
	case 0b000000:
		printf("ec: %b in user space cpu can't fetch instruction because the mmu not map. the arm doc says:Unknown reason. \n", ec);
		break;
	default:
		printf("ec: %b not in handle\n", ec);
		break;
	}
	panic("GET USER TRAP");
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
	// panic("trap message");
}