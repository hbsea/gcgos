#pragma once

struct gicd_regs
{
    int GICD_CTLR;            // 0x000
    int GICD_TYPER;           // 0x004
    int GICD_IIDR;            // 0x008
    int Reserve0[29];         // 0x00C - 0x080
    int GICD_IGROUPR[15];     // 0x080 - 0x0BC
    int Reserve1[17];         // 0x0BC - 0x100
    int GICD_ISENABLER[15];   // 0x100 - 0x13C
    int Reserve2[17];         // 0x13C - 0x180
    int GICD_ICENABLER[15];   // 0x180 - 0x1BC
    int Reserve3[17];         // 0x1BC - 0x200
    int GICD_ISPENDR[15];     // 0x200 - 0x23C
    int Reserve4[17];         // 0x23C - 0x280
    int GICD_ICPENDR[15];     // 0x280 - 0x2BC
    int Reserve5[17];         // 0x2BC - 0x300
    int GICD_ISACTIVER[15];   // 0x300 - 0x33C
    int Reserve6[17];         // 0x33C - 0x380
    int GICD_ICACTIVER[15];   // 0x380 - 0x3BC
    int Reserve7[17];         // 0x3BC - 0x400
    int GICD_IPRIORITYR[127]; // 0x400 - 0x5FC
    int Reserve8[129];        // 0x5FC - 0x800
    int GICD_ITARGETSR[127];  // 0x800 - 0x9FC
    int Reserve9[129];        // 0x9FC - 0xC00
    int GICD_ICFGR[31];       // 0xC00 - 0xC7C
    int Reserve10[33];        // 0xC7C - 0xD00
    int GICD_PPISR;           // 0xD00
    int GICD_SPISR[14];       // 0xD04-0xD3C
    int Reserve11[113];       // 0xD3C - 0xF00
    int GICD_SGIR;            // 0xF00
    int Reserve12[3];      // 0xF04 - 0xF10
    int GICD_CPENDSGIR[4];    // 0xF10 - 0xF1C
    int GICD_SPENDSGIR[4];    // 0xF20 - 0xF2C
    int Reserve13[40];        // 0xF2C - 0xFD0
    int GICD_PIDR4;           // 0xFD0
    int GICD_PIDR5;           // 0xFD4
    int GICD_PIDR6;           // 0xFD8
    int GICD_PIDR7;           // 0xFDC
    int GICD_PIDR0;           // 0xFE0
    int GICD_PIDR1;           // 0xFE4
    int GICD_PIDR2;           // 0xFE8
    int GICD_PIDR3;           // 0xFEC
    int GICD_CIDR0;           // 0xFF0
    int GICD_CIDR1;           // 0xFF4
    int GICD_CIDR2;           // 0xFF8
    int GICD_CIDR3;           // 0xFFC
};

struct gicc_regs
{
    int GICC_CTLR;
    int GICC_PMR;
};

#define gicd ((struct gicd_regs *)GICD_BASE)
#define gicc ((struct gicc_regs *)GICC_BASE)

void write_gicd_sgir(void);
void gic_init(void);