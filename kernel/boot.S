#include "arm.h"
#include "sysregs.h"
//.开头的是汇编器的指示指令，本身不会做任何事情。

// 用于在linker中控制最先运行的启动代码，否则将按文件的字母顺序执行。
//tells the Assembler where to put our code. 
.section ".text.boot"  

.global _sstart  //.global可以在整个工程中使用，如果仅有__start，在本文档中使用，__start$在本段中使用，默认为_start,修改为其他字符串，需要在linker中定义:ENTRY(sometext)
_sstart:
    //starting second core
//    ldr x0, =SPIN_CPU1
//    ldr x1, = master
//    str x1,[x0]
//    sev
//    
    //mrs x0, CurrentEL
    //lsr x0, x0, #2
//
//    mrs x0,mpidr_el1
//    // 根据手册The Cortex-A75 core is single-threaded, so this field has the value 0x00
//    //https://developer.arm.com/documentation/100403/0300/register-descriptions/aarch64-system-registers/mpidr-el1--multiprocessor-affinity-register--el1
//    and x0,x0,#3
//    cbz x0,master

//EL2->EL1 qemu default is EL2
master:
    ldr	x0, =SCTLR_VALUE_MMU_DISABLED
    msr	sctlr_el1, x0

    ldr	x0, =HCR_VALUE
    msr	hcr_el2, x0

	// qemu starts at EL2, than is not possible to access any register of EL3 
    //ldr	x0, =SCR_VALUE
    //msr	scr_el3, x0

    ldr	x0, =SPSR_VALUE
    msr	spsr_el2, x0 	// 'spsr_el3' changed to 'spsr_el2'

    adr	x0, el1_entry		
    msr	elr_el2, x0    // 'elr_el3' changed to 'elr_el2'

    eret    


el1_entry:

    //At EL1, FP/SIMD is disabled by default.
    // Read current CPACR_EL1
    mrs x0, cpacr_el1

    // Set bits 20:23 to 0b11 -> enable full FP/SIMD at EL1
    orr x0, x0, #(0b11 << 20)  // FPEN[21:20]=0b11
    orr x0, x0, #(0b11 << 22)  // FPEN[23:22]=0b11 for EL1
    msr cpacr_el1, x0

    // Optional: ensure all FP/SIMD registers are cleared
    fmov d0, #0

    ldr x0,=stack0
    mov x1, #1024*4
    mrs x2, mpidr_el1
    and x2, x2, #0x3
    add x2, x2, #1
    mul x1, x1, x2
    add x0, x0, x1
    mov sp,x0
    bl kernel_main
    b proc_hang

    
 proc_hang:
     wfi
     b proc_hang

