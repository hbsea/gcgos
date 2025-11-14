#include "proc.h"
#include "defs.h"
#include "memlayout.h"
void exec(struct proc *p)
{
    p->state = RUNNABLE;
};
