#include "proc.h"
#include "param.h"
#include "defs.h"
#include "arm.h"
#include "defs.h"
#include "memlayout.h"

extern char _binary_build_user_user1_start[];
extern char _binary_build_user_user1_size[];

extern char userret[];
extern char trampoline[];
extern char end[];
extern void forkret(void);

struct proc proc[NPROC];
int nextpid = 1;
struct proc* curproc[NCPU];

void proc_mapstacks(pagetable_t kpgtbl)
{
    struct proc* p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        char* pa = kalloc();
        uint64 va = KSTACK((int)(p - proc));
        printf("p:%p KSTACK va:%p\n", p, va);

        kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_NORMAL | PTE_AP_RW_EL1);
        // printf("p-proc:%d\n", p - proc);
    }
}

void procinit(void)
{
    struct proc* p;
    for (p = proc; p < &proc[NPROC]; p++)
    {
        p->state = UNUSED;
        p->kstack = KSTACK((int)(p - proc));
    }
}

int cpuid(void)
{
    int id = r_mpidr_el1();
    return id;
}

int allocpid(void)
{
    int pid;
    pid = nextpid;
    nextpid = nextpid + 1;
    return pid;
}

pagetable_t proc_pagetable(struct proc* p)
{
    pagetable_t pagetable;
    pagetable = uvmcreat();
    mappages(pagetable, TRAMPOLINE, (uint64)trampoline, 2 * PGSIZE,
             PTE_NORMAL | PTE_AP_RW_EL1);
    p->tf = (struct trapframe*)kalloc();
    mappages(pagetable, TRAPFRAME, (uint64)p->tf, PGSIZE,
             PTE_NORMAL | PTE_AP_RW_EL1);

    printf("new user pagetable:%p p->tf:%p \n", pagetable, p->tf);
    return pagetable;
}

struct proc* allocproc(void)
{
    struct proc* p;
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
    struct proc* p;
    p = allocproc();
    curproc[cpuid()] = p;
    printf("curproc TF:%p\n", curproc[cpuid()]->tf);
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
        printf("_binary_user1_bin_size:%p;_binary_user1_bin_start:%p\n",
               _binary_build_user_user1_size, _binary_build_user_user1_start);
        get_entry();
        // uint64 upsize =
        //     PGROUNDUP((uint64)_binary_build_user_user1_size) + PGSIZE;
        //
        // mappages(curproc[cpuid()]->pagetable, 0x0,
        //          (uint64)_binary_build_user_user1_start, upsize,
        //          PTE_NORMAL | PTE_AP_RW);
        //
        // curproc[cpuid()]->tf->elr_el1 =
        //     (uint64)_binary_build_user_user1_start & 0xfff;
        // printf("userproc offset:%p", curproc[cpuid()]->tf->elr_el1);
        // curproc[cpuid()]->tf->sp_el0 = upsize;
    }

    prepare_return();

    uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
    ((void (*)(uint64))trampoline_userret)(
        (uint64)(curproc[cpuid()]->pagetable));

    // asm volatile("eret");
}

struct proc* newproc(void)
{
    struct proc* np;
    np = allocproc();
    np->ppid = curproc[cpuid()]->pid;
    np->tf->kernel_sp = (uint64)(np->kstack + PGSIZE);
    np->ctx.x30 = (uint64)forkret;
    np->tf->x0 = 0;  // so fork() returns 0 in child

    uint64 utext = (uint64)(0);
    printf("utext:%p uproc:%p tf:%p\n", utext, 0, np->tf);
    mappages(np->pagetable, 0x0, utext, PGSIZE, PTE_NORMAL | PTE_AP_RW);
    np->tf->sp_el0 = PGSIZE;
    np->ctx.sp = (uint64)(np->kstack + PGSIZE);

    printf(
        "newproc created,pid is: %d ppid is: %d addr:%p np->ctx.x30:%p "
        "p->ctx.sp:%p\n",
        np->pid, np->ppid, np, np->ctx.x30, np->ctx.sp);
    return np;
}

void sleep(void* chan)
{
    curproc[cpuid()]->chan = chan;
    curproc[cpuid()]->state = WAITING;
    printf("sleeping curproc pid:%d state:%d\n", curproc[cpuid()]->pid,
           curproc[cpuid()]->state);
    sched();
}
void wakeup(void* chan)
{
    struct proc* p;
    if (p->state == WAITING && p->chan == chan)
        for (p = proc; p < &proc[NPROC]; p++) p->state = RUNNABLE;
}

void sched()
{
    // intr_off();
    struct proc *np, *cp;
    static int first = 1;

    cp = curproc[cpuid()];
    while (1)
    {
        for (np = curproc[cpuid()] + 1; np != curproc[cpuid()]; np++)
        {
            if (np == &proc[NPROC])
            {
                printf("NO PROC IS RUNNING\n");
                np = &proc[0];
            }
            if (np->state == RUNNABLE) break;
        }
        if (np->state == RUNNABLE) break;
    }

    // write_gicd_sgir();

    // if (cp == np)
    if (first)
    {
        first = 0;
        printf("cp pid:%d curproc->ctx.x30: %p curproc->tf->sp_el0:%p \n",
               cp->pid, cp->ctx.x30, cp->tf->sp_el0);

        debug();

        asm volatile("mov x30,%0" ::"r"(cp->ctx.x30));
        asm volatile("mov sp,%0" ::"r"(cp->ctx.sp));
        asm volatile("ret");
    }

    printf("old pid:%d curproc->ctx.x30: %p curproc->tf->sp_el0:%p \n", cp->pid,
           cp->ctx.x30, cp->tf->sp_el0);
    curproc[cpuid()] = np;
    printf("c:%p np:%p\n", cp, np);
    printf(
        "swtch : curproc pid:%d pagetable:%p kstack:%p new proc pid: %d "
        "curproc->ctx.x30:%p curproc->ctx.sp:%p\n",
        curproc[cpuid()]->pid, curproc[cpuid()]->pagetable,
        curproc[cpuid()]->kstack, curproc[cpuid()]->pid,
        curproc[cpuid()]->ctx.x30, curproc[cpuid()]->ctx.sp);
    swtch(&cp->ctx, &np->ctx);
}
