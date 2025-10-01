#include "defs.h"
__attribute__((aligned(4096), section("user1_code"))) void uproc1()
{

    // while(1);
    for (int i = 0;; i++)
        ;
    // printf("uproc1:");
    // asm volatile("mov x8,#0x1");
    // asm volatile("svc #0");

    // asm volatile("mov x8,#0x3");
    // asm volatile("svc #0");

    // asm volatile("mov x8,#0x2");
    // asm volatile("svc #0");
}

__attribute__((aligned(4096), section("user2_code"))) void uproc2()
{

    // while(1);
    for (int i = 0;; i++)
        ;
    // printf("uproc1:");
    // asm volatile("mov x8,#0x1");
    // asm volatile("svc #0");

    // asm volatile("mov x8,#0x3");
    // asm volatile("svc #0");

    // asm volatile("mov x8,#0x2");
    // asm volatile("svc #0");
}