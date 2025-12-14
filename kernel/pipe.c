#include "proc.h"
#include "defs.h"
#include "fd.h"

#define PIPESIZE 512
struct pipe
{
    int readopen;
    int writeopen;
    int readp;
    int writep;
    char data[PIPESIZE];
};

int pipe_alloc(struct fd** fd1, struct fd** fd2)
{
    *fd1 = *fd2 = 0;
    struct pipe* p = 0;
    if ((*fd1 = fd_alloc()) == 0) goto opps;
    printf("fdtype:%d\n", (*fd1)->type);
    if ((*fd2 = fd_alloc()) == 0) goto opps;
    p = (struct pipe*)kalloc();
    (*fd1)->type = FD_PIPE;
    (*fd1)->readable = 1;
    (*fd1)->writeable = 0;
    (*fd1)->pipe = p;

    (*fd2)->type = FD_PIPE;
    (*fd2)->readable = 0;
    (*fd2)->writeable = 1;
    (*fd2)->pipe = p;

    return 0;
opps:
    return -1;
}

int pipe_write(struct fd* fd, uint64 addr, int n)
{
    struct proc* p = myproc();
    printf("curproc: %p\n", p);
    char* s = (char*)&addr;
    for (int i = 0; i < n; i++)
    {
        fd->pipe->data[fd->pipe->writep] = (s[i]);
        fd->pipe->writep = fd->pipe->writep + 1;
    }
    return 0;
}

int pipe_read(struct fd* fd, uint64 buf, int n)
{
    struct proc* p = myproc();
    char* s = (char*)buf;
    for (int i = 0; i < n; i++)
    {
        s[i] = fd->pipe->data[fd->pipe->readp];
        fd->pipe->readp = fd->pipe->readp + 1;
    }
    return -1;
}
