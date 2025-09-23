#include "defs.h"
__attribute__((aligned(4096), section("user_code"))) void uproc1()
{

    for (int i = 0; i < 1; i++)
        ;
    // printf("uproc1:");
    asm volatile("mov x8,#0x1");
    asm volatile("svc #0");

    asm volatile("mov x8,#0x3");
    asm volatile("svc #0");

    asm volatile("mov x8,#0x2");
    asm volatile("svc #0");
}
