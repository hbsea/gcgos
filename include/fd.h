#include "param.h"
#include "types.h"
struct fd
{
    enum
    {
        FD_CLOSE,
        FD_NONE,
        FD_PIPE,
        FD_FILE,
    } type;
    int ref;  // reference count
    char readable;
    char writeable;
    struct pipe* pipe;
    struct inode* ip;
    uint off;
};

extern struct fd fds[NFD];
struct fd* fd_alloc();
