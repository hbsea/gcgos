#include "xv6_syscall.h"
#include "proc.h"
#include "defs.h"
#include "param.h"
#include "fd.h"
#include "memlayout.h"

int argraw(int n)
{
    struct proc* p = myproc();
    switch (n)
    {
        case 0:
            return p->tf->x0;
        case 1:
            return p->tf->x1;
        case 2:
            return p->tf->x2;
        case 3:
            return p->tf->x3;
        case 4:
            return p->tf->x4;
        case 5:
            return p->tf->x5;
        case 6:
            return p->tf->x6;
        case 7:
            return p->tf->x7;
    }
    return -1;
}
void argfd(int n, int* ip) { *ip = argraw(n); }
void argaddr(int n, uint64* ip, int max)
{
    uint64 uva = argraw(n);
    uint64 va0 = PGROUNDDOWN(uva);
    struct proc* p = myproc();
    uint64 pa = walkaddr(p->pagetable, va0);
    uint64* argaddr = (uint64*)(pa + uva - va0);

    char* d = (char*)ip;
    char* s = (char*)argaddr;
    while (max-- > 0)
    {
        *d++ = *s++;
    }
}
void argint(int n, int* ip) { *ip = argraw(n); }

int sys_fork(void)
{
    struct proc* p;
    p = newproc();
    // exec(p);
    return p->pid;
}

int sys_exit(void)
{
    proc_exit();
    return 0;
}

int sys_wait(void) { return proc_wait(); }

int sys_cons_putc(void)
{
    uint64 arg1 = argraw(0);
    consputc(arg1);
    return 0;
}

int sys_pipe(void)
{
    struct fd *rfd = 0, *wfd = 0;
    int f1 = -1, f2 = -1;
    struct proc* p = myproc();
    pipe_alloc(&rfd, &wfd);
    f1 = fd_ualloc();
    p->fds[f1] = rfd;
    f2 = fd_ualloc();
    p->fds[f2] = wfd;

    uint64 uva = argraw(0);
    uint64 va0 = PGROUNDDOWN(uva);
    uint64 pa = walkaddr(p->pagetable, va0);
    int* fdp = (int*)(pa + uva - va0);
    fdp[0] = 0;
    fdp[1] = 1;

    return 0;
}
int sys_write(void)
{
    int fd, n;
    uint64 addr;
    argfd(0, &fd);
    argint(2, &n);
    argaddr(1, &addr, n);
    struct proc* p = myproc();

    return fd_write(p->fds[fd], addr, n);
}
int sys_read(void)
{
    int fd, n;
    argfd(0, &fd);
    argint(2, &n);
    struct proc* p = myproc();
    uint64 uva = argraw(1);
    uint64 va0 = PGROUNDDOWN(uva);
    uint64 pa = walkaddr(p->pagetable, va0);
    uint64 buf = (uint64)(pa + uva - va0);
    if (p->fds[fd]->type == FD_PIPE) fd_read(p->fds[fd], buf, n);
    return 0;
}
int sys_close(void)
{
    int fd;
    struct proc* p = myproc();
    argfd(0, &fd);
    fd_close(p->fds[fd]);
    p->fds[fd] = 0;
    return 0;
}
int sys_kill(void)
{
    int pid;
    argint(0, &pid);
    proc_kill(pid);
    return 0;
};
int sys_panic(void) { return 0; }
int sys_cons_puts(void)
{
    char buf[256];
    int i;
    argint(1, &i);
    argaddr(0, (uint64*)&buf, i);

    printf("%s", buf);
    return 0;
}
void syscall(void)
{
    struct proc* cp = myproc();
    int call_num = cp->tf->x8;
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
        case SYS_kill:
            ret = sys_kill();
            break;
        case SYS_panic:
            ret = sys_panic();
            break;
        case SYS_cons_puts:
            ret = sys_cons_puts();
            break;
        default:
            printf("Unknown sys call %d\n", call_num);
            break;
    }
    cp->tf->x0 = ret;
}
