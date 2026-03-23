#include <stdatomic.h>
#include "proc.h"
#include "defs.h"
#include "memlayout.h"
#include "fs.h"
#include "bio.h"
#include "elf.h"
int kexec(char* path)
{
    struct inode* dp;
    struct buf* buf;
    struct elf* user_elf;

    struct proc* p = myproc();
    dp = namei(path);
    if (!dp) panic("kexec file not found\n");
    buf = bread(dp->addrs[0]);

    user_elf = (struct elf*)buf->data;
    if (user_elf->magic != ELF_MAGIC) panic("not an execute file\n");

    struct proghdr* ph;
    uint sz = 0;
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
                data_buf = bread(dp->addrs[i]);
                sec_buf = data_buf;
            }
            else
            {
                data_buf = bread(dp->addrs[31]);
                uint* p = (uint*)data_buf->data;
                if (p[i - 31] == 0) panic("invalid indirect block");
                sec_buf = bread(p[i - 31]);
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
    }
    brelse(buf);
    p->tf->elr_el1 = user_elf->entry;
    p->tf->sp_el0 = PGSIZE;

    return 0;
}
