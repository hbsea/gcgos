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

void pipe_write()
{
    struct proc* p = curproc[cpuid()];
    printf("curproc: %p\n", p);

    int n = p->tf->x2;
    char* s = "xyz";
    // char* s = (char*)p->tf->x1;
    for (int i = 0; i < n; i++)
    {
        p->fds[0]->pipe->data[p->fds[0]->pipe->writep] = s[i];
        p->fds[0]->pipe->writep = p->fds[0]->pipe->writep + 1;
    }
}
