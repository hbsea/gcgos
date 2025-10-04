#include "types.h"
#include "arm.h"
#include "defs.h"
void timerinit(){
   //asm volatile("svc 0");
   
   printf("cntpct_el0:%p\n",r_cntpct_el0());
   printf("cntfrq_el0:%d\n",r_cntfrq_el0());
   uint64 cnt=r_cntpct_el0();
   uint64 interval=10000000;
   w_cntp_cval_el0(cnt+interval);
   w_cntp_ctl_el0(0b01);
   printf("TIMER enabled\n");
}
