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

struct proc
{
    int pid;
    enum
    {
        UNUSED,
        USED,
        RUNNABLE,
        WAITING
    } state;

    pagetable_t pagetable; // User page table
    uint64 sz;
    struct context ctx;

    // uint64 *kstack;
    // struct TRAPFRAME *tf;
};