#include "ulib.h"

char buf[32];
int main()
{
    int pid, fds[2];
    pipe(fds);
    pid = fork();
    if (pid > 0)
    {
        cons_puts("w:\n", 4);
        write(fds[1], "xyz", 4);
    }
    else
    {
        cons_puts("r:\n", 4);
        read(fds[0], buf, sizeof(buf));
        cons_puts(buf, sizeof(buf));
    }
    cons_puts("some text user\n", 16);

    for (;;);
    return 0;
}
