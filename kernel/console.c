#include "defs.h"
#include "dev.h"
#include "spinlock.h"
#include "uart.h"

#define CONSOLE 1

#define BACKSPACE 0x100

struct spinlock console_lock;
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

    acquire(&console_lock);

    for (int i = 0; i < n; i++)
    {
        consputc(b[i]);
    }

    release(&console_lock);
    return n;
}
void consoleinit(void)
{
    pl011_uart_init();
    devsw[CONSOLE].d_write = console_write;
}
