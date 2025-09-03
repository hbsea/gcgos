.global enable_mmu
enable_mmu:
    MSR TTBR0_EL1, X0
    MSR TTBR1_EL1, X1
    MSR TCR_EL1, X2
    MSR MAIR_EL1, X3
    ISB
    MRS X0, SCTLR_EL1
    ORR X0, X0, #1
    MSR SCTLR_EL1, X0
    ISB

    //double map 39bit 
    //high-va:0xFFFF_FF80_0000_0000 ~ 0xFFFFFFFFFFFFFFFF   (512 GB) 
    //low-va:0x0000000000000000 ~ 0x0000007FFFFFFFFF   (512 GB)
    ldr x0,=_step_high
    movz x1, #0xffff, lsl #48   // 高16位
    movk x1, #0xff80, lsl #32   // 中间16位
    movk x1, #0x0000, lsl #16   
    movk x1, #0x0000, lsl #0   

    //high + sp   
     
    mov x2,sp
    orr x2,x2,x1
    mov sp,x2
    
    br x0
    // never come back