int fork()
{
    unsigned long x;
    asm volatile("mov x8,#1");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
void xv6_exit()
{
    asm volatile("mov x8,#2");
    asm volatile("svc #0");
}
void cons_putc(int c)
{
    asm volatile("mov x8,#4");
    asm volatile("svc #0");
}
int pipe(int fd[])
{
    unsigned long x;
    asm volatile("mov x8,#5");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
int write(int fd, char* buf, int n)
{
    unsigned long x;
    asm volatile("mov x8,#6");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
int read(int fd, char* buf, int size)
{
    unsigned long x;
    asm volatile("mov x8,#7");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
void close(int fd)
{
    asm volatile("mov x8,#8");
    asm volatile("svc #0");
}
int block()
{
    unsigned long x;
    asm volatile("mov x8,#9");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
int kill(int pid)
{
    unsigned long x;
    asm volatile("mov x8,#10");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
void cons_puts(char* s, int size)
{
    asm volatile("mov x8,#12");
    asm volatile("svc #0");
}
void exec(char* path, ...)
{
    asm volatile("mov x8,#13");
    asm volatile("svc #0");
}
void puts(char* s)
{
    int i = 0;
    char* ss = s;
    while (*ss++)
    {
        i++;
    }
    cons_puts(s, i);
}

int open(char* s, int n)
{
    unsigned long x;
    asm volatile("mov x8,#14");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}

int mknode(char* s, short type, short major, short minor)
{
    unsigned long x;
    asm volatile("mov x8,#15");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
};

int unlink(char* s)
{
    unsigned long x;
    asm volatile("mov x8,#16");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
}
