#include "types.h"
#include "arm.h"
#include "param.h"
#include "spinlock.h"
struct context
{
    uint64 sp;
    // callee-saved

    /* callee register */
    uint64 x18;
    uint64 x19;  // 0x10
    uint64 x20;  // 0x18
    uint64 x21;  // 0x20
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 x29;
    uint64 x30;
};
struct cpu
{
    struct proc* proc;
    struct context contex;
};
extern struct cpu cpus[NCPU];

struct trapframe
{
    uint64 x0;  // 0x0
    uint64 x1;  // 0x8
    uint64 x2;  // 0x10
    uint64 x3;  // 0x18
    uint64 x4;  // 0x20
    uint64 x5;
    uint64 x6;
    uint64 x7;
    uint64 x8;
    uint64 x9;
    uint64 x10;
    uint64 x11;
    uint64 x12;
    uint64 x13;
    uint64 x14;
    uint64 x15;
    uint64 x16;
    uint64 x17;
    uint64 x18;
    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 x29;
    uint64 x30;          // 0x150
    uint64 x31;          // 0x158
    uint64 kernel_ttbr;  // 0x160
    uint64 kernel_sp;    // 0x168
    uint64 elr_el1;      // 0x170
    uint64 sp_el0;       // 0x178
};

struct proc
{
    struct spinlock lock;
    int pid;
    int ppid;
    enum
    {
        UNUSED,
        RUNNABLE,
        SLEEPING,
        ZOMBIE,
        RUNNING
    } state;
    void* chan;  // sleep channel
    int killed;
    pagetable_t pagetable;  // User page table
    uint64 sz;
    struct context ctx;

    struct fd* fds[NOFILE];

    uint64 kstack;
    struct trapframe* tf;
};

extern struct proc proc[];
extern struct proc* initproc;
