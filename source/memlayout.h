// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x80000L
#define PHYSTOP (KERNBASE + 128*1024*1024)