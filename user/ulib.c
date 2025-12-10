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
void puts(char* s)
{
    for (int i = 0; s[i]; i++)
    {
        cons_putc(s[i]);
    }
}

int pipe(int fd[])
{
    asm volatile("mov x8,#5");
    asm volatile("svc #0");
}
int write(int fd, char* buf, int n)
{
    asm volatile("mov x8,#6");
    asm volatile("svc #0");
}
int read(int fd, char* buf, int size)
{
    asm volatile("mov x8,#7");
    asm volatile("svc #0");
}
