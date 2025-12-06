#include "uart.h"
#include "memlayout.h"
#include "defs.h"

void pl011_uart_init()
{
    gpio->GPFSEL[1] &= ~0x3f000;
    gpio->GPFSEL[1] |= 0x12000;  // set 14 and 15 to alt5 TDX1/RCX1

    gpio->GPIO_PUP_PDN_CNTRL_REG[0] &= ~0xF0000000;
    gpio->GPIO_PUP_PDN_CNTRL_REG[0] |= 0x50000000;  // pull up 14 and 15 pin

    pl011_regs->CR = 0;       // turn off UART0:Control register
    pl011_regs->ICR = 0x7FF;  // clear interrupts:Interupt Clear Register
    pl011_regs->IBRD = 2;     // 115200 baud:Integer Baud rate divisor
    pl011_regs->FBRD = 0xB;   // Fractional Baud rate divisor
    pl011_regs->LCRH =
        0x7 << 4;  // 8n1, enable FIFOs:Line Control register ,  H for high bits
    pl011_regs->CR = 0x301;  // enable Tx, Rx, UART
};
void pl011_uart_send_char(char c)
{
    while (pl011_regs->FR & (1 << 5));
    pl011_regs->DR = c;
};

void pl011_uart_send_text(char* s)
{
    while (*s)
    {
        if (*s == '\n') pl011_uart_send_char('\r');
        pl011_uart_send_char(*s++);
    }
};

int pl011_uart_get_char()
{
    if (pl011_regs->FR & (1 << 4))
    {
        return -1;
    }
    else
    {
        int r = pl011_regs->DR;
        return r == '\r' ? '\n' : r;
    }
}

void pl011_uart_intr()
{
    while (1)
    {
        int c = pl011_uart_get_char();
        if (c == -1)
        {
            break;
        }
        consputc(c);
    }
    pl011_regs->ICR |= (1 << 4);
};

void pl011_uart_ie(void)
{
    // pl011_regs->IFLS =0b010010;
    pl011_regs->IMSC |= (0b1 << 4);
    printf("PL011 enabled:IFLS : %b RIS :%b \n", pl011_regs->IFLS,
           pl011_regs->RIS, &pl011_regs->TDR);
}
