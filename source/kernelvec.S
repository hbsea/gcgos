	.macro	ventry	label
	.align	7
	b	\label
	.endm

.globl kernel_vec
.align 11
kernel_vec:
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

sync_invalid_el1t:		// Synchronous EL1t
irq_invalid_el1t:			// IRQ :L1t
fiq_invalid_el1t:		// FIQ EL1t
error_invalid_el1t:			// Error EL1t

sync_invalid_el1h:			// Synchronous EL1h
 
el1_irq:				// IRQ EL1h
fiq_invalid_el1h:			// FIQ EL1h
error_invalid_el1h:			// Error EL1h

sync_invalid_el0_64:			// Synchronous 64-bit EL0
irq_invalid_el0_64:			// IRQ 64-bit EL0
fiq_invalid_el0_64:			// FIQ 64-bit EL0
error_invalid_el0_64:			// Error 64-bit EL0

sync_invalid_el0_32:			// Synchronous 32-bit EL0
irq_invalid_el0_32:			// IRQ 32-bit EL0
fiq_invalid_el0_32:			// FIQ 32-bit EL0
error_invalid_el0_32:		// Error 32-bit EL0
   b kerneltrap