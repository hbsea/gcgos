#include "ulib.h"
#include "../include/fs.h"
#include "../include/fcntl.h"

char buf[200];
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

    // exec("user2", (char *[]){"user2", 0});
    // char *args[] = {"echo", "hello", "goodbye", 0};
    // exec("/echo", args);  // TODO handle args

    int fd;
    // fd = open("user2");
    // if (fd >= 0)
    // {
    //     puts("open user2 OK\n");
    //     close(fd);
    // }
    // else
    // {
    //     puts("open notexist failed\n");
    // }

    // char *cat_args[] = {"cat", "README", 0};
    // exec("cat", cat_args);
    if (mknode("console", T_DEV, 1, 1) < 0) puts("mknode failed\n");

    int stdout = open("console", O_WRONLY);
    printf(stdout, "user1 running\n");

    fd = open("doesnotexist", O_CREATE | O_RDWR);
    if (fd >= 0)
        printf(stdout, "creat doesnotexist success\n");
    else
        printf(stdout, "create doesnotexist failed\n");

    for (int i = 0; i < 100; i++)
    {
        if (write(fd, "aaaaaaaaaa", 10) != 10)
            printf(stdout, "error:write new file failed\n");
        if (write(fd, "bbbbbbbbbb", 10) != 10)
            printf(stdout, "error:write new file failed\n");
    }

    close(fd);
    fd = open("doesnotexist", O_RDONLY);
    if (fd < 0) printf(stdout, "error:open doesnotexist failed\n");
    int i = read(fd, buf, 10000);
    close(fd);
    for (;;);
    return 0;
}
