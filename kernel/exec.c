#include "proc.h"
#include "defs.h"
#include "memlayout.h"
int kexec(char* path)
{
    namei(path);
    return 0;
}
