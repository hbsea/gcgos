#include "ulib.h"

// char buf[32];
int main()
{
    // int pid, fds[2];
    // pipe(fds);
    // pid = fork();
    // if (pid > 0)
    // {
    //     cons_puts("w:\n", 4);
    //     write(fds[1], "xyz", 4);
    // }
    // else
    // {
    //     cons_puts("r:\n", 4);
    //     read(fds[0], buf, sizeof(buf));
    //     cons_puts(buf, sizeof(buf));
    // }
    puts("user1 running\n");

    // exec("user2", (char *[]){"user2", 0});
    // char *args[] = {"echo", "hello", "goodbye", 0};
    // exec("/echo", args);  // TODO handle args

    int fd;
    fd = open("user2");
    if (fd >= 0)
    {
        puts("open user2 OK\n");
        close(fd);
    }
    fd = open("notexist");
    if (fd >= 0)
    {
        puts("open notexist success\n");
    }
    else
    {
        puts("open notexist failed\n");
    }

    for (;;);
    return 0;
}
