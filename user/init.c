#include "ulib.h"
#include "../include/fcntl.h"
#include "../include/fs.h"
int main()
{
    int stdout, pid;
    if ((stdout = open("console", O_RDONLY)) < 0)
    {
        mknode("console", T_DEV, 1, 1);
        stdout = open("console", O_RDONLY);
    };

    int stdout1 = open("console", O_WRONLY);
    while (1)
    {
        printf(stdout1, "running sh...\n");

        pid = fork();

        if (pid == 0)
        {
            puts("pidis0\n");
            exec("sh", (char*[]){"sh", 0});
            xv6_exit();
        }
        puts("pid larger 0\n");
        for (;;);
    }
}
