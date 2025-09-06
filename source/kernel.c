#include "uart.h"
#include "defs.h"
#include "vm.h"
#include "memlayout.h"

extern char end[];
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

    while (1)
        asm volatile("wfi");
}