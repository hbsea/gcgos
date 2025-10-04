#include "uart.h"
#include "defs.h"
#include "vm.h"
#include "memlayout.h"
#include "proc.h"
#include "gicv2.h"

int kernel_main()
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

    sched();

    return 0;
}
