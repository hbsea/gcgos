#include "defs.h"
#include "param.h"
#include "fd.h"
#include "proc.h"

struct fd fds[NFD];

int fd_ualloc()
{
    int fd;
    struct proc* p = myproc();
    for (fd = 0; fd < NOFILE; fd++)
    {
        if (p->fds[fd] == 0) return fd;
    }
    return -1;
}

struct fd* fd_alloc()
{
    int i;
    for (i = 0; i < NFD; i++)
    {
        if (fds[i].type == FD_CLOSE)
        {
            fds[i].type = FD_NONE;
            fds[i].count = 1;
            return fds + i;
        }
    }
    return 0;
}

int fd_write(struct fd* fd, uint64 addr, int n)
{
    if (fd->type == FD_PIPE) return pipe_write(fd->pipe, addr, n);
    return -1;
}
int fd_read(struct fd* fd, uint64 buf, int n)
{
    if (fd->type == FD_PIPE) return pipe_read(fd->pipe, buf, n);
    return -1;
}
void fd_close(struct fd* fd)
{
    fd->count -= 1;
    if (fd->count == 0)
    {
        if (fd->type == FD_PIPE)
        {
            pipe_close(fd->pipe, fd->writeable);
        }
    }
    fd->type = FD_CLOSE;
}
