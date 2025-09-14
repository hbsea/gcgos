#include "param.h"
#include "proc.h"
#include "defs.h"
#include "arm.h"
#include "defs.h"
#include "memlayout.h"

extern char userret[];
extern char trampoline[];
extern char end[];
extern void forkret(void);
void uservec();
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
    printf("user pagetable:%p\n", pagetable);
    mappages(pagetable, TRAMPOLINE, (uint64)uservec, 2 * PGSIZE, PTE_NORMAL | PTE_AP_RO_EL1);
    p->tf = kalloc();
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
    curproc = initproc = p;
    printf("initproc TF:%p\n", &(curproc->tf));
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

        // fake init user text map,replace it when file ready.
        uint64 utext = (uint64)(&uproc1);
        printf("utext:%p uproc:%p\n", utext, &uproc1);
        mappages(initproc->pagetable, 0x0, utext, PGSIZE, PTE_NORMAL | PTE_AP_RW);

        w_elr_el1(0x0);
    }
    printf("forkret TRAPFRAME:%p\n", TRAPFRAME);
    prepare_return();

    uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);

    debug();

    ((void (*)(uint64))trampoline_userret)((uint64)(TRAPFRAME));
    // asm volatile("eret");
}

struct proc *newproc(void)
{
    struct proc *np;
    np = allocproc();
    np->ppid = curproc->pid;
    np->ctx.sp = (uint64)(np->kstack + PGSIZE);
    np->ctx.x30 = (uint64)forkret;

    printf("newproc created,pid is: %d\ ppid is: %d addr:%p np->ctx.x30:%p p->ctx.sp:%p\n", np->pid, np->ppid, np, np->ctx.x30, np->ctx.sp);
}

void sleep(void *chan)
{
    curproc->chan = chan;
    curproc->state = WAITING;
    printf("sleep curproc pid:%d state:%d\n", curproc->pid, curproc->state);
    swtch();
}
void wakeup(void *chan)
{
    struct proc *p;
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->state = WAITING && p->chan == chan)
            p->state = RUNNABLE;
}

void swtch()
{
    struct proc *np;
    while (1)
    {
        for (np = curproc + 1; np != curproc; np++)
        {
            if (np == &proc[NPROC])
            {
                printf("enum all proc now start from &proc[0]\n");
                np = &proc[0];
            }
            if (np->state == RUNNABLE)
                break;
        }
        if (np->state == RUNNABLE)
            break;
    }

    // asm volatile("mov %0,x30" : "=r"(curproc->ctx.x30));
    // asm volatile("mov %0,sp" : "=r"(curproc->ctx.sp));

    curproc = np;
    printf("curproc PID:%d\n", curproc->pid);
    printf("new proc pid: %d p->ctx.x30:%p p->ctx.sp:%p\n", np->pid, np->ctx.x30, np->ctx.sp);
    debug();
    asm volatile("mov x30,%0" ::"r"(np->ctx.x30));
    asm volatile("mov sp,%0" ::"r"(np->ctx.sp));
    asm volatile("ret");
}
