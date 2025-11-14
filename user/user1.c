void fork()
{
    asm volatile("mov x8,#0x1");
    asm volatile("svc #0");
}
int main()
{
    fork();
    for (int i = 0;; i++)
        ;

    // asm volatile("mov x8,#0x3");
    // asm volatile("svc #0");

    // asm volatile("mov x8,#0x2");
    // asm volatile("svc #0");
    return 0;
}
