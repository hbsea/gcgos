#include "defs.h"
#include "types.h"
#include "elf.h"
#include "memlayout.h"
#include "proc.h"

extern char _binary_build_user_user1_start[];
extern char _binary_build_user_user1_size[];
uint64 get_entry()
{
    struct Elf* uelf;
    uelf = (struct Elf*)_binary_build_user_user1_start;
    printf("bin start:%p\n", _binary_build_user_user1_start);
    printf("uelf:%p,magic:%p\n", uelf, uelf->magic);
    if (uelf->magic != ELF_MAGIC)

    {
        panic("not a elf format");
    }
    struct Proghdr* ph;
    uint64* pa = kalloc();
    for (int i = 0; i < uelf->phnum; i++)
    {
        ph =
            (struct Proghdr*)(_binary_build_user_user1_start + uelf->phoff) + i;
        uint64* ucd = (uint64*)(_binary_build_user_user1_start +
                                i * uelf->phentsize + ph->off);
        for (int s = 0; s < ph->memsz; s++)
        {
            pa[ph->vaddr + s] = ucd[s];
        }
    }
    // uint64 addr = PGROUNDUP(ph->vaddr) + PGSIZE;
    mappages(curproc[cpuid()]->pagetable, ph->vaddr, (uint64)pa, PGSIZE,
             PTE_AP_RW);
    curproc[cpuid()]->tf->elr_el1 = ph->vaddr;
    curproc[cpuid()]->tf->sp_el0 = ph->vaddr + PGSIZE;

    return -1;
}
