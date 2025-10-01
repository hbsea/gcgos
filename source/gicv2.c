#include "memlayout.h"
#include "gicv2.h"
#include "types.h"
#include "defs.h"

#define UART0_IRQ (121+32)//0x79 121

//https://speakerdeck.com/tnishinaga/baremetal-raspberry-pi-4-gicv2
void gic_init()
{

    gicd->GICD_CTLR=0;
	
    uint32 lines=gicd->GICD_TYPER & 0x1F;
    printf(" lines %d\n",(lines+1)*32);
    //for(int i=0;i<lines;i++){
        //gicd->GICD_IGROUPR[i]=~0;
    //}

    // 0. 设置触发方式（Level-sensitive）
    //gicd->GICD_ICFGR[UART0_IRQ / 16] &= ~(2 << ((UART0_IRQ % 16) * 2));
    
    // 1. 使能中断
    gicd->GICD_ISENABLER[UART0_IRQ / 32] = (1 << (UART0_IRQ % 32));

    // gicd->GICD_ISENABLER[0] = 0x0000FFFF; //SGI
   
    // 2. 设置优先级（0 = highest）
    gicd->GICD_IPRIORITYR[UART0_IRQ / 4] &= ~((uint32)0xFF << ((UART0_IRQ % 4) * 8));

    // 3. 设置目标 CPU（假设 CPU0）
    //gicd->GICD_ITARGETSR[UART0_IRQ / 4] |= (1 << ((UART0_IRQ % 4) * 8));
    gicd->GICD_ITARGETSR[UART0_IRQ / 4] &= ~((uint32)0xFF << ((UART0_IRQ % 4) * 8)); // clear old
    gicd->GICD_ITARGETSR[UART0_IRQ / 4] |= ((uint32)0x1 << 0) << ((UART0_IRQ % 4) * 8); // CPU0


    gicc->GICC_CTLR = 0x01;
    gicd->GICD_CTLR = 0x01;
    gicc->GICC_PMR = 0xFF;
    printf("gicd_ctlr:%d gicc_ctlr:%d \n",gicd->GICD_CTLR,gicc->GICC_CTLR);

    asm volatile("msr daifclr,#2");
    asm volatile("dsb sy; isb" ::: "memory");
}

void write_gicd_sgir(void)
{

    gicd->GICD_CTLR = 0x01;

    gicd->GICD_ISENABLER[0] = 0x0000FFFF;

    gicc->GICC_CTLR = 0x01;
    gicc->GICC_PMR = 0xF0;

    asm volatile("msr daifclr,#2");
    uint32 gicd_sgir = ((0x1 & 0xff) << 16) /* CPUTargetList */
                       | ((0u & 0x3) << 24) /* TargetListFilter = 0b00, 这里写 0 */
                       | ((0u & 0x1) << 15) /* SATT = 0 (通常) */
                       | (0x0 & 0xf);       /* SGIINTID */
    printf("GICD_TYPER=%b GICD_IIDR=%b ISPENDR0 = %p SGIR:%p\n", gicd->GICD_TYPER, gicd->GICD_IIDR, gicd->GICD_ISPENDR[0], &gicd->GICD_ISENABLER);
    gicd->GICD_SGIR = gicd_sgir;
    asm volatile("dsb sy; isb" ::: "memory");
}
