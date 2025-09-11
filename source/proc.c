#include "param.h"
#include "proc.h"
#include "defs.h"
#include "arm.h"
#include "defs.h"
#include "memlayout.h"

extern char end[];
extern void forkret(void);
void vectors();
struct proc proc[NPROC];
int nextpid = 1;
struct proc *initproc;

void proc_mapstacks(pagetable_t kpgtbl)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        char *pa = kalloc();
        uint64 va = KSTACK((int)(p - proc));
        // printf("va:%p\n", va);

        kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_NORMAL | PTE_AP_RW_EL1);
        // printf("p-proc:%d\n", p - proc);
    }
}

void procinit(void)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        p->state = UNUSED;
        p->kstack = KSTACK((int)(p - proc));
    }
}

int allocpid(void)
{
    int pid;
    pid = nextpid;
    nextpid = nextpid + 1;
    return pid;
}

pagetable_t proc_pagetable(struct proc *p)
{
    pagetable_t pagetable;
    pagetable = uvmcreat();
    mappages(pagetable, TRAMPOLINE, (uint64)vectors, PGSIZE, PTE_NORMAL | PTE_AP_RO_EL1);
    p->tf = kalloc();
    printf("TRAPFRAME:%p\n",TRAPFRAME);
    mappages(pagetable, TRAPFRAME, (uint64)p->tf, PGSIZE, PTE_NORMAL | PTE_AP_RW_EL1);
    return pagetable;
}

struct proc *allocproc(void)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        if (p->state == UNUSED)
        {
            p->pid = allocpid();
            p->state = RUNNABLE;
            p->pagetable = proc_pagetable(p);
            return p;
        }
    }
}
void userinit(void)
{
    struct proc *p;
    p = allocproc();
    initproc = p;
    p->ctx.sp = (uint64)(p->kstack + PGSIZE);
    p->ctx.x30 = (uint64)forkret;
}

void forkret(void)
{
    static int first = 1;
    if (first)
    {
        first = 0;

        w_sp_el0(PGSIZE);

        uint64 utext = (uint64)(&uproc1);
        printf("utext:%p\ uproc:%p\n", utext, &uproc1);

        mappages(initproc->pagetable, 0x0, utext, PGSIZE, PTE_NORMAL | PTE_AP_RW);
        w_elr_el1(0x0);

        initproc->tf->kernel_pagetable = r_ttbr0_el1();
        w_ttbr0_el1((uint64)(initproc->pagetable));
    }
    prepare_return();
}

void swtch(struct proc *op)
{
    struct proc *np;
    while (1)
    {
        for (np = op + 1; np != op; np++)
        {
            if (np = &proc[NPROC])
                np = &proc[0];
            if (np->state == RUNNABLE)
                break;
        }
        if (np->state == RUNNABLE)
            break;
    }

    asm volatile("mov %0,x30" : "=r"(op->ctx.x30));
    asm volatile("mov %0,sp" : "=r"(op->ctx.sp));

    asm volatile("mov x30,%0" ::"r"(np->ctx.x30));
    asm volatile("mov sp,%0" ::"r"(np->ctx.sp));
    asm volatile("ret");
}
