#include "param.h"
#include "defs.h"
#include "arm.h"
#include "defs.h"
#include "memlayout.h"
#include "proc.h"

extern char userret[];
extern char trampoline[];
extern char end[];
extern void forkret(void);

struct proc proc[NPROC];
int nextpid = 1;
struct proc* initproc;
struct cpu cpus[NCPU];

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
struct cpu* mycpu(void)
{
    int id = cpuid();
    struct cpu* c = &cpus[id];
    return c;
}

struct proc* myproc(void)
{
    struct cpu* c = mycpu();
    struct proc* p = c->proc;
    return p;
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
    initproc = p;
    p->ctx.sp = (uint64)(p->kstack + PGSIZE);
    p->ctx.x30 = (uint64)forkret;
    p->state = RUNNABLE;
    printf("endofuserinit\n");
}

void forkret(void)
{
    struct proc* p = myproc();
    static int first = 1;
    if (first)
    {
        first = 0;
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
    ((void (*)(uint64))trampoline_userret)((uint64)(p->pagetable));

    // asm volatile("eret");
}

struct proc* newproc(void)
{
    struct proc* np;
    struct proc* cp;
    np = allocproc();
    np->ppid = cp->pid;
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
    struct proc* p = myproc();
    p->chan = chan;
    p->state = WAITING;
    printf("sleeping curproc pid:%d state:%d\n", p->pid, p->state);
    scheduler();
}
void wakeup(void* chan)
{
    struct proc* p;
    if (p->state == WAITING && p->chan == chan)
        for (p = proc; p < &proc[NPROC]; p++) p->state = RUNNABLE;
}
void proc_exit()
{
    struct proc* p;
    struct proc* cp = myproc();

    cp->state = ZOMBIE;
    // close fd before exit
    for (int fd; fd < NOFILE; fd++)
    {
        if (cp->fds[fd])
        {
            fd_close(cp->fds[fd]);
            cp->fds[fd] = 0;
        }
    }

    // wakeup parent
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->pid == cp->ppid) wakeup(p);

    // abandon children
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->ppid == cp->pid) p->pid = 1;

    sched();
}

void yield(void)
{
    struct proc* p = myproc();
    p->state = RUNNABLE;
    printf("called yield\n");
    sched();
}
void scheduler()
{
    struct proc* p;
    struct cpu* c = mycpu();
    c->proc = 0;
    while (1)
    {
        intr_on();
        intr_off();
        int found = 0;
        for (p = proc; p < &proc[NPROC]; p++)
        {
            acquire(&p->lock);
            if (p->state == RUNNABLE)
            {
                p->state = RUNNING;
                c->proc = p;
                swtch(&c->contex, &p->ctx);
                c->proc = 0;
                found = 1;
            }
            release(&p->lock);
        }
        printf("found value:%d\n", found);
        if (found == 0)
        {
            printf("wfi cpu %d\n", cpuid());
            asm volatile("wfi");
        }
    }
}

void sched(void)
{
    struct proc* p = myproc();
    swtch(&p->ctx, &mycpu()->contex);
};
