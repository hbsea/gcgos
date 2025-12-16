#include "ulib.h"

char buf[32];
int main()
{
    int pid, fds[2];
    pipe(fds);
    pid = fork();
    if (pid > 0)
    {
        puts("w:");
        write(fds[1], "xyz", 4);
    }
    else
    {
        puts("r:");
        read(fds[0], buf, sizeof(buf));
        puts(buf);
    }

    while (1)
    {
        // for (int i; i < 100000000; i++);
        // puts("&");
    };
    return 0;
}
