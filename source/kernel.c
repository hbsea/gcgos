#include "uart.h"
#include "defs.h"
#include "vm.h"
#include "memlayout.h"
#include "proc.h"

void kernel_main()
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

    swtch(&proc[1]);

    while (1)
        asm volatile("wfi");
}