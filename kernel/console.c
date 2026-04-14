#include "dev.h"
#include "uart.h"

#define CONSOLE 1

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

int console_write(int minor, void *buf, int n)
{
    char *b = buf;
    for (int i = 0; i < n; i++)
    {
        consputc(b[i]);
    }
    return n;
}
void consoleinit(void)
{
    pl011_uart_init();
    devsw[CONSOLE].d_write = console_write;
}
