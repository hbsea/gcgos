#pragma once
#ifndef __ASSEMBLER__
static inline void
w_spsr_el1(uint64 spsr_el1)
{
    asm volatile("msr spsr_el1, %0" ::"r"(spsr_el1));
}
static inline void
w_elr_el1(uint64 elr_el1)
{
    asm volatile("msr elr_el1, %0" ::"r"(elr_el1));
}
static inline void
w_sp_el0(uint64 sp_el0)
{
    asm volatile("msr sp_el0, %0" ::"r"(sp_el0));
}
static inline void
w_ttbr0_el1(uint64 ttbr0_el1)
{
    asm volatile("msr ttbr0_el1, %0" ::"r"(ttbr0_el1));
}

static inline void
w_vbar_el1(uint64 x)
{
    asm volatile("msr vbar_el1, %0" ::"r"(x));
}
static inline uint64
r_vbar_el1(void)
{
    uint64 x;
    asm volatile("mrs %0,vbar_el1" : "=r"(x));
    return x;
}
static inline uint64
r_esr_el1(void)
{
    uint64 x;
    asm volatile("mrs %0,esr_el1" : "=r"(x));
    return x;
}
static inline int
r_CurrentEL(void)
{
    uint64 x;
    asm volatile("mrs %0, CurrentEL" : "=r"(x));
    return x;
}

typedef uint64 pte_t;
typedef uint64 *pagetable_t; // 512 PTE
#endif                       // __ASSEMBLER__

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(sz) ((sz) & ~(PGSIZE - 1))

// PTE[0]
#define PTE_VALID (1 << 0)
// PTE[1]
#define PTE_TABLE (1 << 1)
#define PTE_V 3 // level 3 descriptor: valid

// PTE[4:2] attribute index
// https://developer.arm.com/documentation/ddi0601/2025-06/AArch64-Registers/MAIR-EL1--Memory-Attribute-Indirection-Register--EL1-
// Attr<n>, bits [8n+7:8n], for n = 7 to 0,  index is set by mair_el1[atr8|art7...|atr0]
//  atr0[7...0] Inner memory attributes（Bits[3:0]）：影响核心内部缓存（L1/L2）行为;Outer memory attributes（Bits[7:4]）：影响系统级缓存（共享缓存）行为
#define AI_DEVICE_nGnRnE_IDX 0x0
#define AI_NORMAL_NC_IDX 0x1
// memory type
#define MT_DEVICE_nGnRnE 0x0
#define MT_NORMAL_NC 0xFF

#define PTE_INDX(i) (((i) & 7) << 2)
#define PTE_DEVICE PTE_INDX(AI_DEVICE_nGnRnE_IDX)
#define PTE_NORMAL PTE_INDX(AI_NORMAL_NC_IDX)

// PTE[5:6] PTE_AP(Access Permission) is 2bit field.
//         EL0   EL1
//  00 --   x    RW
//  01 --  RW    RW
//  10 --   x    RO
//  11 --  RO    RO
#define PTE_AP(ap) (((ap) & 3) << 6)
#define PTE_AP_RW_EL1 PTE_AP(0)
#define PTE_AP_RW PTE_AP(1)
#define PTE_AP_RO_EL1 PTE_AP(2)
#define PTE_AP_RO PTE_AP(3)

// PTE[10]  PTE_AF(Access Flag)
//  0 -- this block entry has not yet.
//  1 -- this block entry has been used.
#define PTE_AF (1 << 10)

// PTE[53]Privileged eXecute Never
#define PTE_PXN (1UL << 53)
// PTE[54]Unprivileged eXecute Never
#define PTE_UXN (1UL << 54)
#define PTE_XN (PTE_PXN | PTE_UXN) // eXecute Never

#define PTE_R 1 << 1
#define PTE_W 1 << 2

// shift a physical address to the right place for a PTE.
#define PA2PTE(pa) ((uint64)pa & ~0xFFF)
#define PTE2PA(pte) ((uint64)pte & ~0xFFF)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK 0x1FF // 9 bits
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uint64)(va)) >> PXSHIFT(level)) & PXMASK)

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))

// translation control register
// VA_bits = 39 → TxSZ = 64 - 39 = 25
#define TCR_T0SZ(n) ((n) & 0x3f)
#define TCR_TG0(n) (((n) & 0x3) << 14)
#define TCR_T1SZ(n) (((n) & 0x3f) << 16)
#define TCR_TG1(n) (((n) & 0x3) << 30)
#define TCR_IPS(n) (((uint64)(n) & 0x7) << 32))
