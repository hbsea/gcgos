#include "uart.h"
#include "defs.h"
#include "vm.h"

char txt[] = "hello\n";
void kernel_main()
{
    // mini_uart_init();
    consoleinit();
    kinit();
    kvminit();
    kvminithart();


    while (1)
        ;
}