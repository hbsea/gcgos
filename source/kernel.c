#include "uart.h"
#include "defs.h"

char txt[] = "hello\n";
void kernel_main()
{
    // mini_uart_init();
    // mini_uart_send_text("Hello mini UART!\n");
    pl011_uart_init();
    pl011_uart_send_text("PL011 UART\n");
    pl011_uart_send_text(txt);

    kinit();
    void *pa = kalloc();
    printf("new pa addr is at: %p \n", pa);

    while (1)
    {
    };
}