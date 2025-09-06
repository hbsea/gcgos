#include "param.h"
#include "proc.h"
#include "defs.h"
#include "arm.h"
#include "defs.h"
#include "memlayout.h"

extern char end[];
struct proc proc[NPROC];
int nextpid = 1;
void procinit(void)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        p->state = UNUSED;
    }
}

int allocpid(void)
{
    int pid;
    pid = nextpid;
    nextpid = nextpid + 1;
    return pid;
}

pagetable_t proc_pagetable()
{
    pagetable_t pagetable;
    pagetable = uvmcreat();
    uint64 utext = PGROUNDDOWN((uint64)&uproc1);
    printf("utext: %p \n", utext);
    if (mappages(pagetable, KERNBASE, KERNBASE, (uint64)end - KERNBASE, PTE_NORMAL | PTE_AP_RO) != 0)
        panic("utext map");
    return pagetable;
}

struct proc *allocproc()
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        if (p->state == UNUSED)
        {
            p->pid = allocpid();
            p->state = USED;
            p->pagetable = proc_pagetable();
            uint64 ustack = (uint64)kalloc() + PGSIZE;
            printf("ustack:%p\n", ustack);
            if (mappages(p->pagetable, ustack - PGSIZE, ustack - PGSIZE, PGSIZE, PTE_NORMAL | PTE_AP_RW) != 0)
                panic("ustack");

            el1_el0(0b0000, (uint64)&uproc1, ustack, (uint64)p->pagetable);
            return p;
        }
    }
}
void userinit(void)
{
    struct proc *p;
    p = allocproc();
}
// void swtch(struct proc *op)
// {
//     struct proc *np;
//     while (1)
//     {
//         for (np = op + 1; np != op; np++)
//         {
//             if (np = &proc[NPROC])
//                 np = &proc[0];
//             if (np->state == RUNNABLE)
//                 break;
//         }
//         if (np->state == RUNNABLE)
//             break;
//     }

// }