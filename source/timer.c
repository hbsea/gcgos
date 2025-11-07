#include "types.h"
#include "defs.h"
#include "arm.h"
// https://tc.gts3.org/cs3210/2020/spring/r/aarch64-generic-timer.pdf
void timerinit()
{
    // asm volatile("svc 0");

    printf("cntpct_el0:%lu\n", r_cntpct_el0());
    printf("cntfrq_el0:%d\n", r_cntfrq_el0());
    uint64 cnt = r_cntpct_el0();
    uint64 interval = 10000000;
    w_cntp_cval_el0(cnt + interval);
    w_cntp_ctl_el0(0b01);
    printf("TIMER enabled\n");
}
