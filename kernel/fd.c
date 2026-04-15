#include "defs.h"
#include "param.h"
#include "fd.h"
#include "proc.h"
#include "dev.h"

struct devsw devsw[NDEV];
struct fd fds[NFD];

int fd_ualloc(void)
{
    int fd;
    struct proc* p = myproc();
    for (fd = 0; fd < NOFILE; fd++)
    {
        if (p->fds[fd] == FD_CLOSE) return fd;
    }
    return -1;
}

struct fd* fd_alloc(void)
{
    int i;
    for (i = 0; i < NFD; i++)
    {
        if (fds[i].type == FD_CLOSE)
        {
            fds[i].type = FD_NONE;
            fds[i].ref = 1;
            return fds + i;
        }
    }
    return 0;
}

int fd_write(struct fd* fd, uint64 addr, int n)
{
    if (fd->writeable == 0) return -1;
    if (fd->type == FD_PIPE) return pipe_write(fd->pipe, addr, n);
    if (fd->type == FD_FILE)
    {
        int r = writei(fd->ip, (void*)addr, fd->off, n);
        if (r > 0)
        {
            fd->off += r;
        }
        return r;
    }
    else
    {
        panic("fd_write");
        return -1;
    }
}
int fd_read(struct fd* fd, uint64 buf, int n)
{
    if (fd->readable == 0) return -1;
    if (fd->type == FD_PIPE) return pipe_read(fd->pipe, buf, n);
    if (fd->type == FD_FILE)
    {
        int cc = readi(fd->ip, (void*)buf, fd->off, n);
        if (cc > 0) fd->off += cc;
        return cc;
    }
    return -1;
}
void fd_close(struct fd* fd)
{
    fd->ref--;
    if (fd->ref == 0)
    {
        if (fd->type == FD_PIPE)
        {
            pipe_close(fd->pipe, fd->writeable);
        }
        if (fd->type == FD_FILE)
        {
            file_close(fd->ip);
        }
    }
    fd->type = FD_CLOSE;
}

void fd_incref(struct fd* fd) { fd->ref++; }
