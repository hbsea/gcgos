#include "types.h"
#include "arm.h"
struct context
{
    uint64 sp;
    // callee-saved

    /* callee register */
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
    uint64 x30;
};

struct trapframe
{
    uint64 kernel_ttbr; // 0x0
    uint64 kernel_sp;   // 0x8
    uint64 x0;          // 0x10
    uint64 x1;          // 0x18
    uint64 x2;          // 0x20
    uint64 x3;
    uint64 x4;
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
    uint64 x30;
};

struct proc
{
    int pid;
    int ppid;
    enum
    {
        UNUSED,     
        RUNNABLE,
        WAITING,
        ZOMBIE
    } state;
    void *chan;
    pagetable_t pagetable; // User page table
    uint64 sz;
    struct context ctx;

    uint64 kstack;
    struct trapframe *tf;
};

extern struct proc proc[];
extern struct proc *curproc;
