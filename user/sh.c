#include "ulib.h"

int main()
{
    int pid;
    char buf[128];
    write(1, "sh run now\n", 11);
    for (;;)
    {
        write(1, "$ ", 2);
        gets(buf, sizeof(buf));
        if (buf[0] == '\0') continue;
        write(1, buf, 14);
        pid = fork();
        if (pid == 0)
        {
            exec(buf, (char*[]){buf, 0});
            // exec("llbb", (char*[]){"llbb", 0});
            for (;;);
            xv6_exit();
        }
        if (pid > 0) wait();
    }
}
