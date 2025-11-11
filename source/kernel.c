#include "types.h"
#include "arm.h"
#include "defs.h"
#include "gicv2.h"
#include "memlayout.h"
#include "proc.h"
#include "uart.h"
#include "vm.h"
#include "param.h"

volatile static int started = 0;

int kernel_main()
{
    if (cpuid() == 0)
    {
        // mini_uart_init();
        consoleinit();

        kinit();
        kvminit();
        kvminithart();
        procinit();
        trapinit();
        trapinithart();
        userinit();
        sys_fork();
        gic_init();
        pl011_uart_ie();
        timerinit();

        __sync_synchronize();
        started = 1;
    }
    else
    {
        while (started == 0)
            ;
        __sync_synchronize();
        printf("cpuid %d starting\n", cpuid());
        kvminithart();
        trapinit();
        gic_init();
    }

    sched();

    return 0;
}
