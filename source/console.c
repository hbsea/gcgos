#include "uart.h"

#define BACKSPACE 0x100

void consputc(int c)
{
    if (c == BACKSPACE)
    {
        pl011_uart_send_char('\b');
        pl011_uart_send_char(' ');
        pl011_uart_send_char('\b');
    }
    else
    {
        pl011_uart_send_char(c);
    }
}