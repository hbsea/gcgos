#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "sdhci.h"

// https://github.com/bztsrc/raspi3-tutorial/blob/master/0B_readsector/sd.c

// SCR (SD Configuration Register)
// RCA (Relative Card Address)
// OCR (Operating Condition Register)
uint64 sd_scr[2], sd_rca, sd_err, sd_hv;

/**
 * Wait N CPU cycles (ARM CPU only)
 */
void wait_cycles(unsigned int n)
{
    if (n)
        while (n--)
        {
            asm volatile("nop");
        }
}

/**
 * Wait N microsec (ARM CPU only)
 */
void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile("mrs %0, cntpct_el0" : "=r"(t));
    // calculate required count increase
    unsigned long i = ((f / 1000) * n) / 1000;
    // loop while counter increase is less than i
    do
    {
        asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while (r - t < i);
}

/**
 * Wait for data or command ready
 */
int sd_status(unsigned int mask)
{
    int cnt = 500000;
    while ((sdchi->STATUS & mask) && !(sdchi->INTERRUPT & INT_ERROR_MASK) &&
           cnt--)
        wait_msec(1);
    return (cnt <= 0 || (sdchi->INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

/**
 * Wait for interrupt
 */
int sd_int(unsigned int mask)
{
    unsigned int r, m = mask | INT_ERROR_MASK;
    int cnt = 1000000;
    while (!(sdchi->INTERRUPT & m) && cnt--) wait_msec(1);
    r = sdchi->INTERRUPT;
    if (cnt <= 0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT))
    {
        sdchi->INTERRUPT = r;
        return SD_TIMEOUT;
    }
    else if (r & INT_ERROR_MASK)
    {
        sdchi->INTERRUPT = r;
        return SD_ERROR;
    }
    sdchi->INTERRUPT = mask;
    return 0;
}

/**
 * Send a command
 */
int sd_cmd(unsigned int code, unsigned int arg)
{
    int r = 0;
    sd_err = SD_OK;
    if (code & CMD_NEED_APP)
    {
        r = sd_cmd(CMD_APP_CMD | (sd_rca ? CMD_RSPNS_48 : 0), sd_rca);
        if (sd_rca && !r)
        {
            printf("ERROR: failed to send SD APP command\n");
            sd_err = SD_ERROR;
            return 0;
        }
        code &= ~CMD_NEED_APP;
    }
    if (sd_status(SR_CMD_INHIBIT))
    {
        printf("ERROR: EMMC busy\n");
        sd_err = SD_TIMEOUT;
        return 0;
    }
    printf("EMMC: Sending command %p arg %p\n", code, arg);
    sdchi->INTERRUPT = sdchi->INTERRUPT;
    sdchi->ARG1 = arg;
    sdchi->CMDTM = code;
    if (code == CMD_SEND_OP_COND)
        wait_msec(1000);
    else if (code == CMD_SEND_IF_COND || code == CMD_APP_CMD)
        wait_msec(100);
    if ((r = sd_int(INT_CMD_DONE)))
    {
        printf("ERROR: failed to send EMMC command\n");
        sd_err = r;
        return 0;
    }
    r = sdchi->RESP0;
    if (code == CMD_GO_IDLE || code == CMD_APP_CMD)
        return 0;
    else if (code == (CMD_APP_CMD | CMD_RSPNS_48))
        return r & SR_APP_CMD;
    else if (code == CMD_SEND_OP_COND)
        return r;
    else if (code == CMD_SEND_IF_COND)
        return r == arg ? SD_OK : SD_ERROR;
    else if (code == CMD_ALL_SEND_CID)
    {
        r |= sdchi->RESP3;
        r |= sdchi->RESP2;
        r |= sdchi->RESP1;
        return r;
    }
    else if (code == CMD_SEND_REL_ADDR)
    {
        sd_err = (((r & 0x1fff)) | ((r & 0x2000) << 6) | ((r & 0x4000) << 8) |
                  ((r & 0x8000) << 8)) &
                 CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }
    return r & CMD_ERRORS_MASK;
    // make gcc happy
    return 0;
}

/**
 * read a block from sd card and return the number of bytes read
 * returns 0 on error.
 */
int sd_readblock(unsigned int lba, unsigned char* buffer, unsigned int num)
{
    int r, c = 0, d;
    if (num < 1) num = 1;
    printf("sd_readblock lba %p num %p\n", lba, num);
    if (sd_status(SR_DAT_INHIBIT))
    {
        sd_err = SD_TIMEOUT;
        return 0;
    }
    unsigned int* buf = (unsigned int*)buffer;
    if (sd_scr[0] & SCR_SUPP_CCS)
    {
        if (num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT))
        {
            sd_cmd(CMD_SET_BLOCKCNT, num);
            if (sd_err) return 0;
        }
        sdchi->BLKSIZECNT = (num << 16) | 512;
        sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI, lba);
        if (sd_err) return 0;
    }
    else
    {
        sdchi->BLKSIZECNT = (1 << 16) | 512;
    }
    while (c < num)
    {
        if (!(sd_scr[0] & SCR_SUPP_CCS))
        {
            sd_cmd(CMD_READ_SINGLE, (lba + c) * 512);
            if (sd_err) return 0;
        }
        if ((r = sd_int(INT_READ_RDY)))
        {
            printf("\rERROR: Timeout waiting for ready to read\n");
            sd_err = r;
            return 0;
        }
        for (d = 0; d < 128; d++) buf[d] = sdchi->DATA;
        c++;
        buf += 128;
    }
    if (num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) &&
        (sd_scr[0] & SCR_SUPP_CCS))
        sd_cmd(CMD_STOP_TRANS, 0);
    return sd_err != SD_OK || c != num ? 0 : num * 512;
}

/**
 * write a block to the sd card and return the number of bytes written
 * returns 0 on error.
 */
int sd_writeblock(unsigned char* buffer, unsigned int lba, unsigned int num)
{
    int r, c = 0, d;
    if (num < 1) num = 1;
    printf("sd_writeblock lba %d  num %d \n", lba, num);
    if (sd_status(SR_DAT_INHIBIT | SR_WRITE_AVAILABLE))
    {
        sd_err = SD_TIMEOUT;
        return 0;
    }
    unsigned int* buf = (unsigned int*)buffer;
    if (sd_scr[0] & SCR_SUPP_CCS)
    {
        if (num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT))
        {
            sd_cmd(CMD_SET_BLOCKCNT, num);
            if (sd_err) return 0;
        }
        sdchi->BLKSIZECNT = (num << 16) | 512;
        sd_cmd(num == 1 ? CMD_WRITE_SINGLE : CMD_WRITE_MULTI, lba);
        if (sd_err) return 0;
    }
    else
    {
        sdchi->BLKSIZECNT = (1 << 16) | 512;
    }
    while (c < num)
    {
        if (!(sd_scr[0] & SCR_SUPP_CCS))
        {
            sd_cmd(CMD_WRITE_SINGLE, (lba + c) * 512);
            if (sd_err) return 0;
        }
        if ((r = sd_int(INT_WRITE_RDY)))
        {
            printf("\rERROR: Timeout waiting for ready to write\n");
            sd_err = r;
            return 0;
        }
        for (d = 0; d < 128; d++) sdchi->DATA = buf[d];
        c++;
        buf += 128;
    }
    if ((r = sd_int(INT_DATA_DONE)))
    {
        printf("\rERROR: Timeout waiting for data done\n");
        sd_err = r;
        return 0;
    }
    if (num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) &&
        (sd_scr[0] & SCR_SUPP_CCS))
        sd_cmd(CMD_STOP_TRANS, 0);
    return sd_err != SD_OK || c != num ? 0 : num * 512;
}

/**
 * set SD clock to frequency in Hz
 */
int sd_clk(unsigned int f)
{
    unsigned int d, c = 41666666 / f, x, s = 32, h = 0;
    int cnt = 100000;
    while ((sdchi->STATUS & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && cnt--)
        wait_msec(1);
    if (cnt <= 0)
    {
        printf("ERROR: timeout waiting for inhibit flag\n");
        return SD_ERROR;
    }

    sdchi->CONTROL1 &= ~C1_CLK_EN;
    wait_msec(10);
    x = c - 1;
    if (!x)
        s = 0;
    else
    {
        if (!(x & 0xffff0000u))
        {
            x <<= 16;
            s -= 16;
        }
        if (!(x & 0xff000000u))
        {
            x <<= 8;
            s -= 8;
        }
        if (!(x & 0xf0000000u))
        {
            x <<= 4;
            s -= 4;
        }
        if (!(x & 0xc0000000u))
        {
            x <<= 2;
            s -= 2;
        }
        if (!(x & 0x80000000u))
        {
            x <<= 1;
            s -= 1;
        }
        if (s > 0) s--;
        if (s > 7) s = 7;
    }
    if (sd_hv > HOST_SPEC_V2)
        d = c;
    else
        d = (1 << s);
    if (d <= 2)
    {
        d = 2;
        s = 0;
    }
    printf("sd_clk divisor %d shift %d\n", d, s);
    if (sd_hv > HOST_SPEC_V2) h = (d & 0x300) >> 2;
    d = (((d & 0x0ff) << 8) | h);
    sdchi->CONTROL1 = (sdchi->CONTROL1 & 0xffff003f) | d;
    wait_msec(10);
    sdchi->CONTROL1 |= C1_CLK_EN;
    wait_msec(10);
    cnt = 10000;
    while (!(sdchi->CONTROL1 & C1_CLK_STABLE) && cnt--) wait_msec(10);
    if (cnt <= 0)
    {
        printf("ERROR: failed to get stable clock\n");
        return SD_ERROR;
    }
    return SD_OK;
}

int sd_init(void)
{
    long r, cnt, ccs = 0;
    printf("SDCHI is %p \n", &sdchi->SLOTISR_VER);
    sd_hv = (sdchi->SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
    // Reset the card.
    sdchi->CONTROL0 = 0;
    sdchi->CONTROL1 |= C1_SRST_HC;
    cnt = 10000;
    do
    {
        wait_msec(10);
    } while ((sdchi->CONTROL1 & C1_SRST_HC) && cnt--);
    if (cnt <= 0)
    {
        printf("ERROR: failed to reset EMMC\n");
        return SD_ERROR;
    }
    printf("EMMC: reset OK\n");
    sdchi->CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
    wait_msec(10);
    // Set clock to setup frequency.
    if ((r = sd_clk(400000))) return r;
    sdchi->INT_EN = 0xffffffff;
    sdchi->INT_MASK = 0xffffffff;
    sd_scr[0] = sd_scr[1] = sd_rca = sd_err = 0;
    sd_cmd(CMD_GO_IDLE, 0);
    if (sd_err) return sd_err;

    sd_cmd(CMD_SEND_IF_COND, 0x000001AA);
    if (sd_err) return sd_err;
    cnt = 6;
    r = 0;
    while (!(r & ACMD41_CMD_COMPLETE) && cnt--)
    {
        wait_cycles(400);
        r = sd_cmd(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        printf("EMMC: CMD_SEND_OP_COND returned ");
        if (r & ACMD41_CMD_COMPLETE) printf("COMPLETE ");
        if (r & ACMD41_VOLTAGE) printf("VOLTAGE ");
        if (r & ACMD41_CMD_CCS) printf("CCS %p %p", r >> 32, r);
        printf("\n");
        if (sd_err != SD_TIMEOUT && sd_err != SD_OK)
        {
            printf("ERROR: EMMC ACMD41 returned error\n");
            return sd_err;
        }
    }
    if (!(r & ACMD41_CMD_COMPLETE) || !cnt) return SD_TIMEOUT;
    if (!(r & ACMD41_VOLTAGE)) return SD_ERROR;
    if (r & ACMD41_CMD_CCS) ccs = SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID, 0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR, 0);
    printf("EMMC: CMD_SEND_REL_ADDR returned %p %p\n", sd_rca >> 32, sd_rca);
    if (sd_err) return sd_err;

    if ((r = sd_clk(25000000))) return r;

    sd_cmd(CMD_CARD_SELECT, sd_rca);
    if (sd_err) return sd_err;

    if (sd_status(SR_DAT_INHIBIT)) return SD_TIMEOUT;
    sdchi->BLKSIZECNT = (1 << 16) | 8;
    sd_cmd(CMD_SEND_SCR, 0);
    if (sd_err) return sd_err;
    if (sd_int(INT_READ_RDY)) return SD_TIMEOUT;

    r = 0;
    cnt = 100000;
    while (r < 2 && cnt)
    {
        if (sdchi->STATUS & SR_READ_AVAILABLE)
            sd_scr[r++] = sdchi->DATA;
        else
            wait_msec(1);
    }
    if (r != 2) return SD_TIMEOUT;
    if (sd_scr[0] & SCR_SD_BUS_WIDTH_4)
    {
        sd_cmd(CMD_SET_BUS_WIDTH, sd_rca | 2);
        if (sd_err) return sd_err;
        sdchi->CONTROL0 |= C0_HCTL_DWITDH;
    }
    // add software flag
    printf("EMMC: supports ");
    if (sd_scr[0] & SCR_SUPP_SET_BLKCNT) printf("SET_BLKCNT ");
    if (ccs) printf("CCS ");
    printf("\n");
    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;
    return SD_OK;
}
