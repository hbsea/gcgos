#include "uart.h"
#include "defs.h"
#include "vm.h"
#include "memlayout.h"
#include "proc.h"

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


    sched();

    return 0;
}
