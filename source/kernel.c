#include "uart.h"

void kernel_main()
{
    uart_init();
    uart_send_text("Hello \nWorld!");
    while (1)
    {
    };
}