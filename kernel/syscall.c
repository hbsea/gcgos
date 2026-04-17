#include "fs.h"
#include "xv6_syscall.h"
#include "proc.h"
#include "defs.h"
#include "param.h"
#include "fd.h"
#include "memlayout.h"
#include "fcntl.h"

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
    while (max-- > 0) *d++ = *s++;
}

void fetchaddr(uint64 uva, uint64* ip, int size)
{
    uint64 va0 = PGROUNDDOWN(uva);
    struct proc* p = myproc();
    uint64 pa = walkaddr(p->pagetable, va0);
    uint64* argaddr = (uint64*)(pa + uva - va0);

    char* d = (char*)ip;
    char* s = (char*)argaddr;
    while (*s && size-- > 0) *d++ = *s++;
}
void argint(int n, int* ip) { *ip = argraw(n); }

int sys_fork(void)
{
    struct proc *cp, *np;
    cp = myproc();

    np = copyproc(cp);
    np->state = RUNNABLE;
    // p = newproc();
    // exec(p);
    return np->pid;
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
    char buf[512];
    argfd(0, &fd);
    argint(2, &n);
    argaddr(1, (uint64*)&buf, n);
    struct proc* p = myproc();

    return fd_write(p->fds[fd], (uint64)buf, n);
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
    return fd_read(p->fds[fd], buf, n);
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
    for (int x = 0; x < 256; x++) buf[x] = 0;

    int i;
    argint(1, &i);
    argaddr(0, (uint64*)&buf, i);

    printf("%s", buf);
    return 0;
}

int sys_exec(void)
{
    char exe_name[DIRSIZ];
    for (int x = 0; x < DIRSIZ; x++) exe_name[x] = 0;
    argaddr(0, (uint64*)&exe_name, DIRSIZ);

    char args[512];
    argaddr(1, (uint64*)&args, 512);
#define MAXARG 32
    uint64* argv[MAXARG];
    for (int i = 0; i < MAXARG; i++)
    {
        argv[i] = 0;
    }
    uint64* arg = (uint64*)args;

    for (int i = 0; arg[i] != 0; i++)
    {
        argv[i] = kalloc();
        fetchaddr(arg[i], argv[i], PGSIZE);
    }

    kexec(exe_name, (char**)&argv);

    return 0;
}
int sys_open(void)
{
    char file_name[DIRSIZ];
    int arg1, rootdev = 1;
    struct inode* dp;

    for (int x = 0; x < DIRSIZ; x++) file_name[x] = 0;
    argaddr(0, (uint64*)&file_name, DIRSIZ);

    argint(1, &arg1);
    struct inode* ip = namei(file_name);

    if (ip == 0)
    {
        if (arg1 & O_CREATE)
        {
            dp = iget(rootdev, 1);
            if (dp->type != T_DIR) return -1;
            ip = mknod(dp, file_name, T_FILE, 0, 0);
            iput(dp);
            if (ip == 0) return -1;
        }
        else
            return -1;
    }

    int ufd;
    struct fd* fd_file;
    fd_file = fd_alloc();
    ufd = fd_ualloc();
    struct proc* p = myproc();

    iunlock(ip);
    fd_file->type = FD_FILE;
    fd_file->ip = ip;
    fd_file->off = 0;
    if (arg1 & O_RDWR)
    {
        fd_file->readable = 1;
        fd_file->writeable = 1;
    }
    else if (arg1 & O_WRONLY)
    {
        fd_file->readable = 0;
        fd_file->writeable = 1;
    }
    else
    {
        fd_file->readable = 1;
        fd_file->writeable = 0;
    }

    p->fds[ufd] = fd_file;

    return ufd;
}
int sys_mknod(void)
{
    char dev_name[DIRSIZ];
    for (int x = 0; x < DIRSIZ; x++) dev_name[x] = 0;
    argaddr(0, (uint64*)&dev_name, DIRSIZ);
    int type_dev;
    argint(1, &type_dev);
    int major;
    argint(2, &major);
    int minor;
    argint(3, &minor);

    int rootdev = 1;
    struct inode *dp, *nip;
    dp = iget(rootdev, 1);

    nip = mknod(dp, dev_name, T_DEV, major, minor);
    if (nip == 0) return -1;
    iput(dp);
    iput(nip);

    return 0;
}

int sys_unlink(void)
{
    struct inode* ip;
    char file_name[DIRSIZ];
    for (int x = 0; x < DIRSIZ; x++) file_name[x] = 0;
    argaddr(0, (uint64*)&file_name, DIRSIZ);

    ip = namei(file_name);
    ip->nlink--;
    if (ip->nlink <= 0) panic("sys_unlink: unimplement");

    iupdate(ip);
    iput(ip);

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
        case SYS_close:
            ret = sys_close();
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
        case SYS_exec:
            ret = sys_exec();
            break;
        case SYS_open:
            ret = sys_open();
            break;
        case SYS_mknod:
            ret = sys_mknod();
            break;
        case SYS_unlink:
            ret = sys_unlink();
            break;
        default:
            printf("Unknown sys call %d\n", call_num);
            panic("syscall");
            break;
    }
    cp->tf->x0 = ret;
}
