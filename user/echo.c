#include "ulib.h"
int main(int argc, char* argv[])
{
    puts("echo running now\n");
    for (int i; i < argc; i++)
    {
        puts(argv[i]);
        puts(" ");
    }
    puts("\n");
    for (;;);
    xv6_exit();
}
