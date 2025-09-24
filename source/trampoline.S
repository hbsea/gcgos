#include "trap.h"
#include "memlayout.h"

/* VBAR_ELx
                Synchronous|IRQ       |FIQ       |SError    |
-----+----------+----------+----------+----------+----------+
ELx->ELx SP_EL0 |0x000     |0x080     |0x100     |0x080     |
-----+----------+----------+----------+----------+----------+
ELx->ELx SP_ELx |0x200     |0x280     |0x300     |0x380     |
-----+----------+----------+----------+----------+----------+
ELx->ELy AArch64|0x400     |0x480     |0x500     |0x580     |
-----+----------+----------+----------+----------+----------+
ELx->ELy AArch32|0x600     |0x680     |0x700     |0x780     |
*/




	.macro handle_invalid_entry type
    
	//msr spsel,#0x0 //当trap进入el1后，硬件自动将sp设置使用sp_el1.如果使用select sp_el0，通过设置spsel=#0x0.是可以使用，但是如果尝试重写sp_el0时,会无限循环进trap。
	//sub sp,sp,#0x10 //arm64没有riscv的sscratch(Software Scratch Register),仅使用sp_el0会污染user stack
	//str x0,[sp]

	mov sp, x0
	mrs x0, CurrentEL

	ldr x0, =TRAPFRAME	//现在还是user的pagetable,所以TRAPFRAME可以访问。
	// x0-x5 args x8 syscall_num

	ldr x0,[x0]
	
	ldr x0, =TRAPFRAME
	stp x0, x1, [x0]	
	stp x2, x3, [x0,#0x10]
	stp x4, x5, [x0,#0x20]
	stp x6, x7, [x0,#0x30]
	stp x8, x9, [x0,#0x40]
	ldp x8, x9, [x0,#0x40]
	stp x10, x11, [x0,#0x50]
	stp x12, x13, [x0,#0x60]
	stp x14, x15, [x0,#0x70]
	stp x16, x17, [x0,#0x80]
	stp x18, x19, [x0,#0x90]
	stp x20, x21, [x0,#0xa0]
	stp x22, x23, [x0,#0xb0]
	stp x24, x25, [x0,#0xc0]
	stp x26, x27, [x0,#0xd0]
	stp x28, x29, [x0,#0xe0]
	stp x30, x31, [x0,#0xf0]
	ldp x0, x1, [x0]
	mov x1, sp
	str x1, [x0]

	ldp x1,x2,[x0,#0x100] //kerenl_pagetable kernel_stack

	dsb ishst
	tlbi vmalle1is
	dsb ish
	msr ttbr0_el1,x1	//正试切换至kernel的kpgtble。进程初始化时需要设置proc->ctx->kernel_pagetable.
	dsb ishst
	tlbi vmalle1is
	dsb ish
	isb

	mov sp,x2


	// x0=TRAPFRAME
	mov	x1, #\type
	mrs	x2, esr_el1
	mrs	x3, elr_el1
	mov x4, x8
	ldr x5,=show_invalid_entry_message
	blr	x5

	ldr x0,=usertrap
	blr x0
  

	//user return userret()

	b userret

	.endm



	.macro	ventry	label
	.align	7
	b	\label
	.endm
	
.section trampsec, "ax", %progbits //userret  non-ABS relocatio
.globl trampoline
trampoline:

.globl uservec 
.align 11
uservec:
    // call the C trap handler in trap.c
   	ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	ventry	sync_invalid_el1h			// Synchronous EL1h
	ventry	el1_irq					// IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	ventry	sync_invalid_el0_64			// Synchronous 64-bit EL0
	ventry	irq_invalid_el0_64			// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64			// Error 64-bit EL0

	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32			// Error 32-bit EL0




sync_invalid_el1t:
	handle_invalid_entry  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry  SYNC_INVALID_EL1h

el1_irq:
	handle_invalid_entry  IRQ_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	//b kerneltrap
	handle_invalid_entry  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_invalid_entry  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  ERROR_INVALID_EL0_32


.section userretsec
.globl userret
userret:
	dsb ishst
	tlbi vmalle1is
	dsb ish
	msr ttbr0_el1, x0  //切换到user的pagetable
	dsb ishst
	tlbi vmalle1is
	dsb ish
	isb

	ldr x0,=TRAPFRAME
	ldp x2, x3, [x0,#0x10]
	ldp x4, x5, [x0,#0x20]
	ldp x6, x7, [x0,#0x30]
	ldp x8, x9, [x0,#0x40]
	ldp x10, x11, [x0,#0x50]
	ldp x12, x13, [x0,#0x60]
	ldp x14, x15, [x0,#0x70]
	ldp x16, x17, [x0,#0x80]
	ldp x18, x19, [x0,#0x90]
	ldp x20, x21, [x0,#0xa0]
	ldp x22, x23, [x0,#0xb0]
	ldp x24, x25, [x0,#0xc0]
	ldp x26, x27, [x0,#0xd0]
	ldp x28, x29, [x0,#0xe0]
	ldp x30, x31, [x0,#0xf0]
	ldp x0, x1, [x0,#0x0]	
	eret
		
