#include "ulib.h"
char buf[513];
int main(int argc, char *argv[])
{
    int cc, fd;
    if (argc < 2)
    {
        puts("Usage: cat files...\n");
        xv6_exit();
    }

    for (int i = 1; i < argc; i++)
    {
        fd = open(argv[i], 0);
        if (fd < 0)
        {
            puts("open file failed\n");
            xv6_exit();
        }
        while ((cc = read(fd, buf, sizeof(buf) - 1)) > 0)
        {
            buf[cc] = '\0';
            puts(buf);
        }
        close(fd);
    }
    xv6_exit();
    return 0;
}
