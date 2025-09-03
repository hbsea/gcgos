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
    __asm__(".global _step_high\n_step_high:");

    printf("high\n");

    while (1)
        ;
}