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
    ret
