#include "proc.h"
#include "defs.h"
#include "memlayout.h"
#include "fs.h"
#include "bio.h"
#include "elf.h"
int first = 1;
int kexec(char* path, char** args)
{
    struct inode* dp;
    struct buf* buf;
    struct elf* user_elf;
    struct proc* p;

    if (first)
    {
        first = 0;
        p = myproc();
    }
    else
    {
        p = newproc();
    }

    dp = namei(path);
    if (!dp) panic("kexec file not found\n");
    buf = bread(dp->dev, dp->addrs[0]);

    user_elf = (struct elf*)buf->data;
    if (user_elf->magic != ELF_MAGIC) panic("not an execute file\n");

    struct proghdr* ph;
    uint64 sz = 0;
    for (int ph_i = 0; ph_i < user_elf->phnum; ph_i++)
    {
        ph = (struct proghdr*)((char*)user_elf + user_elf->phoff) + ph_i;
        if (ph->type != ELF_PROG_LOAD) continue;
        // Disk -> Mem -> mmu
        int file_start_index = ph->off / 512;
        int file_end_index = (ph->off + ph->filesz) / 512;

        uint64* pa = kalloc();  // WIP: supose not larger than one Pagezise
        for (int i = file_start_index, j = 0; i <= file_end_index; j++, i++)
        {
            struct buf *data_buf, *sec_buf;
            if (i <= 30)
            {
                data_buf = bread(dp->dev, dp->addrs[i]);
                sec_buf = data_buf;
            }
            else
            {
                data_buf = bread(dp->dev, dp->addrs[31]);
                uint* p = (uint*)data_buf->data;
                if (p[i - 31] == 0) panic("invalid indirect block");
                sec_buf = bread(dp->dev, p[i - 31]);
            }
            char* s = (char*)sec_buf->data;
            char* d = (char*)pa;

            for (int x = 0; x < (512 / sizeof(char)); x++)
            {
                d[x + j * 512] = s[x];
            }
            brelse(data_buf);
            brelse(sec_buf);
        }

        mappages(p->pagetable, ph->vaddr, (uint64)pa, PGSIZE, PTE_AP_RW);
        sz = ph->vaddr;
    }
    sz = PGROUNDUP(sz) + PGSIZE;
    uint64* sp = kalloc();
    char* dsp = (char*)sp + PGSIZE;

    int argc;
    for (argc = 0; args[argc] != 0; argc++);

    uint64 ustack[argc + 1];  //+1 for NULL

    int s;

    uint64 align_sz = sz + PGSIZE;
    for (s = argc - 1; s >= 0; s--)
    {
        uint len;
        char* arg = args[s];
        for (len = 0; arg[len] != 0; len++);
        dsp -= len;
        // align stack
        dsp = (char*)((uint64)dsp & ~0xF);
        align_sz = (align_sz - len - 1) & ~0xF;
        ustack[s] = align_sz;

        char* d_dsp = dsp;
        for (int c = 0; c < len; c++) *d_dsp++ = *arg++;
        *d_dsp++ = '\0';
    }
    ustack[argc] = 0;
    dsp = (char*)((uint64)(dsp - argc * sizeof(uint64)) & ~0xF);
    align_sz = (align_sz - argc * sizeof(uint64)) & ~0xF;
    for (int u = 0; u < argc + 1; u++)
    {
        uint64* d_dsp = (uint64*)dsp;
        d_dsp[u] = ustack[u];
    }

    mappages(p->pagetable, sz, (uint64)sp, PGSIZE, PTE_AP_RW);

    p->tf->sp_el0 = (uint64)align_sz;
    p->tf->x0 = argc;
    p->tf->x1 = (uint64)align_sz;

    p->tf->elr_el1 = user_elf->entry;
    p->state = RUNNABLE;

    brelse(buf);
    return 0;
}
