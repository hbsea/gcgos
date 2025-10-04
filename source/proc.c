#include "param.h"
#include "proc.h"
#include "defs.h"
#include "arm.h"
#include "defs.h"
#include "memlayout.h"
#include "gicv2.h"

extern char userret[];
extern char trampoline[];
extern char end[];
extern void forkret(void);

struct proc proc[NPROC];
int nextpid = 1;
struct proc *initproc;
struct proc *curproc;

void proc_mapstacks(pagetable_t kpgtbl)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        char *pa = kalloc();
        uint64 va = KSTACK((int)(p - proc));
        printf("p:%p KSTACK va:%p\n", p, va);

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
    mappages(pagetable, TRAMPOLINE, (uint64)trampoline, 2 * PGSIZE, PTE_NORMAL | PTE_AP_RW_EL1);
    p->tf = (struct trapframe *)kalloc();
    mappages(pagetable, TRAPFRAME, (uint64)p->tf, PGSIZE, PTE_NORMAL | PTE_AP_RW_EL1);

    printf("new user pagetable:%p p->tf:%p \n", pagetable, p->tf);
    return pagetable;
}

struct proc *allocproc(void)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        printf("p:%p\n", p);
        if (p->state == UNUSED)
        {
            p->pid = allocpid();
            p->state = RUNNABLE;
            p->pagetable = proc_pagetable(p);
            return p;
        }
    }
    return 0;
}

void userinit(void)
{
    struct proc *p;
    p = allocproc();
    curproc = initproc = p;
    printf("initproc TF:%p\n", curproc->tf);
    debug();
    p->ctx.sp = (uint64)(p->kstack + PGSIZE);
    p->ctx.x30 = (uint64)forkret;
}

void forkret(void)
{
    static int first = 1;
    if (first)
    {
        first = 0;

        // fake init user text map,replace it when file ready.
        uint64 utext = (uint64)(&uproc1);
        // printf("utext:%p uproc:%p\n", utext, &uproc1);

        mappages(curproc->pagetable, 0x0, utext, PGSIZE, PTE_NORMAL | PTE_AP_RW);
        curproc->tf->sp_el0 = PGSIZE;
    }

    prepare_return();

    uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
    ((void (*)(uint64))trampoline_userret)((uint64)(curproc->pagetable));

    // asm volatile("eret");
}

struct proc *newproc(void)
{
    struct proc *np;
    np = allocproc();
    np->ppid = curproc->pid;
    np->tf->kernel_sp = (uint64)(np->kstack + PGSIZE);
    np->ctx.x30 = (uint64)forkret;

    uint64 utext = (uint64)(&uproc2);
    printf("utext:%p uproc:%p tf:%p\n", utext, &uproc2, np->tf);

    mappages(np->pagetable, 0x0, utext, PGSIZE, PTE_NORMAL | PTE_AP_RW);

    np->tf->sp_el0 = PGSIZE;
    np->ctx.sp = (uint64)(np->kstack + PGSIZE);

    printf("newproc created,pid is: %d\ ppid is: %d addr:%p np->ctx.x30:%p p->ctx.sp:%p\n", np->pid, np->ppid, np, np->ctx.x30, np->ctx.sp);
    return np;
}

void sleep(void *chan)
{
    curproc->chan = chan;
    curproc->state = WAITING;
    printf("sleeping curproc pid:%d state:%d\n", curproc->pid, curproc->state);
    sched();
}
void wakeup(void *chan)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->state == WAITING && p->chan == chan)
            p->state = RUNNABLE;
}

void sched()
{

    //intr_off();
    struct proc *np, *cp;
    static int first = 1;

    cp = curproc;
    while (1)
    {
        for (np = curproc + 1; np != curproc; np++)
        {
            if (np == &proc[NPROC])
            {
                printf("NO PROC IS RUNNING\n");
                np = &proc[0];
            }
            if (np->state == RUNNABLE)
                break;
        }
        if (np->state == RUNNABLE)
            break;
    }

    //write_gicd_sgir();

    // if (cp == np)
    if (first)
    {

        first = 0;
        printf("cp pid:%d curproc->ctx.x30: %p curproc->tf->sp_el0:%p \n", cp->pid, cp->ctx.x30, cp->tf->sp_el0);

	debug();

        asm volatile("mov x30,%0" ::"r"(cp->ctx.x30));
        asm volatile("mov sp,%0" ::"r"(cp->ctx.sp));
        asm volatile("ret");
    }

    printf("old pid:%d curproc->ctx.x30: %p curproc->tf->sp_el0:%p \n", cp->pid, cp->ctx.x30, cp->tf->sp_el0);
    curproc = np;
    printf("c:%p np:%p\n", cp, np);
    printf("swtch : curproc pid:%d pagetable:%p kstack:%p new proc pid: %d curproc->ctx.x30:%p curproc->ctx.sp:%p\n", curproc->pid, curproc->pagetable, curproc->kstack, curproc->pid, curproc->ctx.x30, curproc->ctx.sp);
    swtch(&cp->ctx, &np->ctx);
}
