#include "proc.h"
#include "defs.h"
#include "syscall.h"
#include "param.h"

void sys_fork(void)
{
    newproc();
    debug();
}

void sys_exit(void)
{
    struct proc *p;

    curproc->state = ZOMBIE;

    // wakeup parent
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->pid == curproc->ppid)
            wakeup(p);

    // abandon children
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->ppid == curproc->pid)
            p->pid = 1;

    swtch();
}

void sys_wait(void)
{
    struct proc *p;
    int any = 0;
    printf("wait pid :%d ppid:%d \n", curproc->pid, curproc->ppid);
    while (1)
    {
        for (p = proc; p < &proc[NPROC]; p++)
        {
            if (p->state == ZOMBIE && p->ppid == curproc->pid)
            {
                kfree(p->tf);
                kfree(p->pagetable);
                p->state = UNUSED;
                prinf("%x collected %x \n", curproc, p);
                return;
            }
        }
        if (p->state != UNUSED && p->ppid == curproc->pid)
            any = 1;
        if (any)
        {
            printf("nothing to wait for\n", curproc);
            return;
        }
        sleep(curproc);
    }
}

void syscall(void)
{
    printf("syscall NUM:%p\n", curproc->tf->x8);
    int call_num = curproc->tf->x8;
    switch (call_num)
    {
    case SYS_fork:
        printf("sys_call_fork\n");
        sys_fork();
        break;
    case SYS_exit:
        printf("sys_call_exit\n");
        sys_exit();
        break;
    case SYS_wait:
        printf("sys_call_wait\n");
        sys_wait();
        break;
    default:
        printf("Unknown sys call %d\n", call_num);
        break;
    }
}