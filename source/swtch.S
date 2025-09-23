.globl swtch
swtch:

    stp x19, x20, [x0, #0x10]
    stp x21, x22, [x0, #0x20]
    stp x23, x24, [x0, #0x30]
    stp x25, x26, [x0, #0x40]
    stp x27, x28, [x0, #0x50]
    stp x29, x30, [x0, #0x60]
    mov x19, sp
    stp x19, x18, [x0]


 
    ldp x18, x19,[x1]
    mov sp, x18
    ldp x18, x19,[x1]
    ldp x19, x20, [x1, #0x10]
    ldp x21, x22, [x1, #0x20]
    ldp x23, x24, [x1, #0x30]
    ldp x25, x26, [x1, #0x40]
    ldp x27, x28, [x1, #0x50]
    ldp x29, x30, [x1, #0x60]

    ret
    
