#include "ulib.h"
int main()
{
    int pid, fds[2];
    pipe(fds);
    pid = fork();
    write(fds[1], "xyz", 4);
    puts("\nw\n");
    while (1);
    return 0;
}
