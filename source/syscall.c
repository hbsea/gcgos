#include "proc.h"
#include "defs.h"
#include "syscall.h"

void sys_fork(void)
{
    newproc();
}

void sys_exit(void)
{
    curproc->state = UNUSED;
    swtch();
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
    default:
        printf("Unknown sys call %d\n", call_num);
        break;
    }
}