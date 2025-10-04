// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x80000L
#define PHYSTOP (KERNBASE + 128 * 1024 * 1024)

// UART
#define PERIPHERAL_BASE 0xFE000000
#define GPIO_BASE (PERIPHERAL_BASE | 0x00200000)
// hardware is (GPIO_BASE|0x00210000)
#define AUX_BASE (GPIO_BASE | 0x15000)
#define PL011_BASE (GPIO_BASE | 0x1000)
#define PL011_IRQ (121+32)//0x79 121
#define TIMER_IRQ 30

#define GIC_BASE 0xFF840000
#define GICD_BASE (GIC_BASE + 0x1000)
#define GICC_BASE (GIC_BASE + 0x2000)

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(sz) ((sz) & ~(PGSIZE - 1))

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
// Trampoline
#define TRAMPOLINE MAXVA - 2 * PGSIZE

// Trapframe
#define TRAPFRAME TRAMPOLINE - PGSIZE

#define KSTACK(p) TRAMPOLINE - (((p) + 1) * 2 * PGSIZE)
