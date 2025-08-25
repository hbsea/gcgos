
#include "uart.h"
void mini_uart_init()
{
    gpio->GPFSEL[1] &= ~0x3f000;
    gpio->GPFSEL[1] |= 0x12000; // set 14 and 15 to alt5 TDX1/RCX1

    gpio->GPIO_PUP_PDN_CNTRL_REG[0] &= ~0xF0000000;
    gpio->GPIO_PUP_PDN_CNTRL_REG[0] |= 0x50000000; // pull up 14 and 15 pin

    aux_mu_regs->AUX_ENABLES = 1;

    aux_mu_regs->AUX_MU_IER_REG = 0;
    aux_mu_regs->AUX_MU_CNTL_REG = 0;

    aux_mu_regs->AUX_MU_LCR_REG = 3; // data 8 bit
    aux_mu_regs->AUX_MU_MCR_REG = 0;

    aux_mu_regs->AUX_MU_IIR_REG = 0xC6; // C for enable FIFO,6 for clear t/r FIFO
    aux_mu_regs->AUX_MU_BAUD_REG = 541; //(clock/(buad*8)-1) clock=500000000
    aux_mu_regs->AUX_MU_CNTL_REG = 3;   // enable TDX/RCX
};
void mini_uart_send_char(char c)
{
    while (!(aux_mu_regs->AUX_MU_LSR_REG & 0x20))
        ;
    aux_mu_regs->AUX_MU_IO_REG = c;
};

void mini_uart_send_text(char *s)
{
    while (*s)
    {
        if (*s == '\n')
            mini_uart_send_char('\r');
        mini_uart_send_char(*s++);
    }
};

int mini_uart_recev()
{

    if (aux_mu_regs->AUX_MU_LSR_REG & 0x01)
    {
        int text = aux_mu_regs->AUX_MU_IO_REG;
        return text;
    }
    else
    {
        return -1;
    }
};

