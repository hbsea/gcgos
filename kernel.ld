/*用于指定程序入口地址，如果没有这行，默认程序入口是_start，否则编译器会报错：warning: cannot find entry symbol _start; not setting start address*/
ENTRY(_sstart) 

SECTIONS
{
    /*aarch64起始地址，可以是0x80000,也可以是0x10000，linux使用的是后者。此处，点表示当前地址，后面必须跟空格，结尾必须分号*/
    . = 0x80000; 
    /*代码段起始地址*/
    .text : { 
      *(.text.boot) *(.text*);   
      . = ALIGN(0x1000); 
      *(trampsec)
      PROVIDE(userret_code = .); 
      *(userretsec)
      . = ALIGN(0x1000);
      PROVIDE(user_code = .); 
      *(user_code)
      . = ALIGN(0x1000);
      PROVIDE(etext = .); 
    } 
    .rodata : { *(.rodata*)}
    .data : { *(.data*)}
    .bss : { *(.bss*)}

    . = ALIGN(0x1000); 
    PROVIDE(end = .);
}
