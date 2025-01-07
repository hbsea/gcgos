//.开头的是汇编器的指示指令，本身不会做任何事情。

// 用于在linker中控制最先运行的启动代码，否则将按文件的字母顺序执行。
//tells the Assembler where to put our code. 
.section ".text.boot"  

.global _sstart  // Execution starts here

_sstart:
    //将act led灯打开
    //外设基地址
    ldr x4,=0xfe000000
    //gpio基地址
    orr x4,x4,#0x00200000
    //设置act led，在pi4上是gpio42
    ldr x5,=0x00000400
    str x5,[x4,0x20] //GPSET1