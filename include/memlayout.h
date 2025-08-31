// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x80000L
#define PHYSTOP (KERNBASE + 128 * 1024 * 1024)


//UART
#define PERIPHERAL_BASE 0xFE000000
#define GPIO_BASE (PERIPHERAL_BASE | 0x00200000)
// hardware is (GPIO_BASE|0x00210000)
#define AUX_BASE (GPIO_BASE | 0x15000)
#define PL011_BASE (GPIO_BASE | 0x1000)