int fork()
{
    unsigned long x;
    asm volatile("mov x8,#0x1");
    asm volatile("svc #0");
    asm volatile("mov %0,x0" : "=r"(x));
    return x;
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
    return x;
}
int write(int fd, char* buf, int n)
{
    unsigned long x;
    asm volatile("mov x8,#6");
    asm volatile("svc #0");
    return x;
}
int read(int fd, char* buf, int size)
{
    unsigned long x;
    asm volatile("mov x8,#7");
    asm volatile("svc #0");
    return x;
}
int close(int fd)
{
    unsigned long x;
    asm volatile("mov x8,#8");
    asm volatile("svc #0");
    return x;
}
int block()
{
    unsigned long x;
    asm volatile("mov x8,#9");
    asm volatile("svc #0");
    return x;
}
int kill(int pid)
{
    unsigned long x;
    asm volatile("mov x8,#10");
    asm volatile("svc #0");
    return x;
}
void cons_puts(char* s, int size)
{
    asm volatile("mov x8,#12");
    asm volatile("svc #0");
}
