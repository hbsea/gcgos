#include "xv6_syscall.h"
#include "proc.h"
#include "defs.h"
#include "param.h"
#include "fd.h"

// int fetchint() { return 1; }
int fetcharg()
{
    struct proc* p = curproc[cpuid()];
    printf("user page table:%p\n", p->pagetable);
    return 0;
}

int sys_fork(void)
{
    struct proc* p;
    p = newproc();
    // exec(p);
    return p->pid;
}

int sys_exit(void)
{
    struct proc* p;

    curproc[cpuid()]->state = ZOMBIE;

    // wakeup parent
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->pid == curproc[cpuid()]->ppid) wakeup(p);

    // abandon children
    for (p = proc; p < &proc[NPROC]; p++)
        if (p->ppid == curproc[cpuid()]->pid) p->pid = 1;

    sched();
    return 0;
}

int sys_wait(void)
{
    struct proc* p;
    int any, pid;
    printf("wait pid :%d ppid:%d \n", curproc[cpuid()]->pid,
           curproc[cpuid()]->ppid);
    while (1)
    {
        any = 0;
        for (p = proc; p < &proc[NPROC]; p++)
        {
            if (p->state == ZOMBIE && p->ppid == curproc[cpuid()]->pid)
            {
                kfree(p->tf);
                kfree(p->pagetable);
                p->state = UNUSED;
                printf("%x collected %x \n", curproc[cpuid()], p);
                return pid;
            }
        }
        if (p->state != UNUSED && p->ppid == curproc[cpuid()]->pid) any = 1;
        if (any == 0)
        {
            printf("nothing to wait for\n", curproc[cpuid()]);
            return -1;
        }
        sleep(curproc[cpuid()]);
    }
}

int sys_cons_putc()
{
    uint64 arg1 = curproc[cpuid()]->tf->x0;
    consputc(arg1);
    return 0;
}

int sys_pipe()
{
    struct fd *rfd = 0, *wfd = 0;
    int f1 = -1, f2 = -1;
    struct proc* p = curproc[cpuid()];
    pipe_alloc(&rfd, &wfd);
    f1 = fd_ualloc();
    p->fds[f1] = rfd;
    f2 = fd_ualloc();
    p->fds[f2] = wfd;
    return 0;
}
int sys_write()
{
    struct proc* p = curproc[cpuid()];
    fetcharg();
    if (p->fds[p->tf->x0]->type == FD_PIPE) pipe_write();

    return 0;
}
int sys_read() { return 0; }
void syscall(void)
{
    int call_num = curproc[cpuid()]->tf->x8;
    // printf("curproc->pid:%d call_num:%d\n", curproc->pid, curproc->tf->x8);
    int ret = -1;
    switch (call_num)
    {
        case SYS_fork:
            ret = sys_fork();
            break;
        case SYS_exit:
            ret = sys_exit();
            break;
        case SYS_wait:
            ret = sys_wait();
            break;
        case SYS_cons_putc:
            ret = sys_cons_putc();
            break;
        case SYS_pip:
            ret = sys_pipe();
            break;
        case SYS_write:
            ret = sys_write();
            break;
        case SYS_read:
            ret = sys_read();
            break;
        default:
            printf("Unknown sys call %d\n", call_num);
            break;
    }
    curproc[cpuid()]->tf->x0 = ret;
}
