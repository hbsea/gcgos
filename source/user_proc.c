#include "defs.h"
__attribute__((aligned(4096))) void uproc1(void)
{

    for (int i = 0; i < 10; i++)
        printf("uproc1:");
}
