//.开头的是汇编器的指示指令，本身不会做任何事情。

// 用于在linker中控制最先运行的启动代码，否则将按文件的字母顺序执行。
//tells the Assembler where to put our code. 
.section ".text.boot"  

.global _sstart  //.global可以在整个工程中使用，如果仅有__start，在本文档中使用，__start$在本段中使用，默认为_start,修改为其他字符串，需要在linker中定义:ENTRY(sometext)
_sstart:
    mrs x0,mpidr_el1
    // 根据手册The Cortex-A75 core is single-threaded, so this field has the value 0x00
    //https://developer.arm.com/documentation/100403/0300/register-descriptions/aarch64-system-registers/mpidr-el1--multiprocessor-affinity-register--el1
    and x0,x0,#3
    cbz x0,stage2
stage1:
    wfe
    b stage1

stage2:
    ldr x0,=_sstart
    mov sp,x0
    bl kernel_main
    b stage1

    