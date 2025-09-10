
.globl alltrap

alltrap:
    mrs x0, esr_el1
    lsr x0,x0,#26
    and x0,x0,#0x3f


.globl trapret
trapret:
    eret