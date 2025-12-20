#pragma once

struct gpio_struct
{
    int GPFSEL[7];
    int GPSET[2];
    int GPCLR[2];
    int GPLEV[2];
    int GPEDS[2];
    int GPREN[2];
    int GPFEN[2];
    int GPHEN[2];
    int GPLEN[2];
    int GPAREN[2];
    int GPAFEN[2];
    int GPIO_PUP_PDN_CNTRL_REG[3];
};

struct aux_mu_reg
{
    int AUX_IRQ;
    int AUX_ENABLES;
    int Reserve[14];
    int AUX_MU_IO_REG;
    int AUX_MU_IER_REG;
    int AUX_MU_IIR_REG;
    int AUX_MU_LCR_REG;
    int AUX_MU_MCR_REG;
    int AUX_MU_LSR_REG;
    int AUX_MU_MSR_REG;
    int AUX_MU_SCRATCH;
    int AUX_MU_CNTL_REG;
    int AUX_MU_STAT_REG;
    int AUX_MU_BAUD_REG;
};
struct pl011_reg
{
    int DR;
    int RSRECR[5];
    int FR;
    int Reserv1;
    int ILPR;
    int IBRD;
    int FBRD;
    int LCRH;
    int CR;
    int IFLS;
    int IMSC;
    int RIS;
    int MIS;
    int ICR;
    int DMACR;
    int Rserve2[15];
    int ITCR;
    int ITIP;
    int ITOP;
    int TDR;
};

#define gpio ((struct gpio_struct*)GPIO_BASE)
#define aux_mu_regs ((struct aux_mu_reg*)AUX_BASE)
#define pl011_regs ((struct pl011_reg*)PL011_BASE)

void mini_uart_init(void);
void mini_uart_send_char(char c);
void mini_uart_send_text(char* s);
int mini_uart_recev(void);
void pl011_uart_init(void);
void pl011_uart_send_char(char c);
void pl011_uart_send_text(char* s);
void pl011_uart_intr(void);
void pl011_uart_ie(void);
