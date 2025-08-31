#include "defs.h"
#include "memlayout.h"
#include "arm.h"

pagetable_t kernel_pagetable;
extern char etext[]; // kernel.ld sets this to end of kernel code.
pagetable_t kvmmake(void)
{
    pagetable_t kpgtbl, tpgtbl;
    kpgtbl = (pagetable_t)kalloc();
    printf("kpgtbl : %p \n", kpgtbl);
    printf("etext:%p\n", etext);
    
    // uart registers
    kvmmap(kpgtbl, PL011_BASE, PL011_BASE, PGSIZE, PTE_DEVICE | PTE_XN);
    // map kernel text executable and read-only.
    kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_NORMAL | PTE_AP_RO_EL1);
    // map kernel data and the physical RAM we'll make use of.
    kvmmap(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP - (uint64)etext, PTE_NORMAL | PTE_XN);
    return kpgtbl;
}

void kvmmap(pagetable_t kpgtble, uint64 va, uint64 pa, uint64 sz, uint64 perm)
{
    if (mappages(kpgtble, va, pa, sz, perm) != 0)
        panic("kvmmap");
}

void kvminit(void)
{
    kernel_pagetable = kvmmake();
}

// Switch the current CPU's h/w page table register to
// the kernel's page table, and enable paging.
void kvminithart()
{
    uint64 ttbr0 = (uint64)kernel_pagetable;
    uint64 ttbr1 = (uint64)0;
    uint64 tcr = (TCR_T0SZ(25) | TCR_T1SZ(25) | TCR_TG0(0) | TCR_TG1(0) | TCR_IPS(0));
    uint mair = ((MT_DEVICE_nGnRnE << (8 * AI_DEVICE_nGnRnE_IDX)) | (MT_NORMAL_NC << (8 * AI_NORMAL_NC_IDX)));
    enable_mmu(ttbr0, ttbr1, tcr, mair);
    printf("enabled mmu\n");
}

pte_t *walk(pagetable_t pagetable, uint64 va, int alloc)
{

    if (va >= MAXVA)
        panic("walk");

    for (int level = 2; level > 0; level--)
    {
        pte_t *pte = &pagetable[PX(level, va)];
        if ((*pte & PTE_VALID) && (*pte & PTE_TABLE))
        {
            pagetable = (pagetable_t)PTE2PA(*pte);
        }
        else
        {
            if (!alloc || (pagetable = (pte_t *)kalloc()) == 0)
                return 0;
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, va)];
}

int mappages(pagetable_t pagetable, uint64 va, uint64 pa, uint64 size, uint64 perm)
{
    uint64 a, last;
    pte_t *pte;
    if ((va % PGSIZE) != 0)
        panic("mappages: va not aligned");

    // 要在ld中4k对齐
    if ((size % PGSIZE) != 0)
        panic("mappages: size not aligned");

    if (size == 0)
        panic("mappages: size");
    a = va;
    last = va + size - PGSIZE;
    for (;;)
    {
        if ((pte = walk(pagetable, a, 1)) == 0)
            return -1;
        if (*pte & PTE_AF)
            panic("mappages:remap");
        *pte = PA2PTE(pa) | perm | PTE_V | PTE_AF;

        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}