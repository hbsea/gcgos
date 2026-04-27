#include "defs.h"
#include "dev.h"
#include "spinlock.h"
#include "uart.h"

struct
{
#define INPUT_BUF_SIZE 128
    char buf[INPUT_BUF_SIZE];
    uint r;
    uint w;
    uint e;

} cons;

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

int console_read(int minor, char *dst, int n)
{
    // printf("cosnole_read cons.r:%p\n", &cons.r);
    while (cons.r == cons.w) sleep(&cons.r);
    for (int i = 0; i < n && cons.buf[i] != '\0'; i++)
    {
        *dst = cons.buf[cons.r++ % INPUT_BUF_SIZE];
        consputc(*dst & 0xff);
        dst++;
        if (cons.r > INPUT_BUF_SIZE) cons.r = 0;
    }
    return n;
}

void consoleintr(char c)
{
    // intr_on();
    // for (;;);
    cons.buf[cons.w++ % INPUT_BUF_SIZE] = c;
    printf("constoleintr cons.r:%p, c: %d\n", &cons.r, c);
    if (c == '\n' || c == '\r') wakeup(&cons.r);
}
void consoleinit(void)
{
    pl011_uart_init();
    devsw[CONSOLE].d_write = console_write;
    devsw[CONSOLE].d_read = console_read;
}
