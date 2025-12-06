#include "defs.h"
#include "param.h"
#include "fd.h"
#include "proc.h"

struct fd fds[NFD];

int fd_ualloc()
{
    int fd;
    struct proc* p = curproc[cpuid()];
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
