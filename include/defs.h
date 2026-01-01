#pragma once
#include "types.h"
#include "arm.h"
#include "spinlock.h"

// console.c
void consoleinit(void);
void consputc(int);

// printf.c
int printf(char*, ...) __attribute__((format(printf, 1, 2)));
void panic(char*);

// kalloc.c
void kinit(void);
void kfree(void*);
void* kalloc(void);
void countkmem(void);

// vm.c
void kvminit(void);
void kvminithart(void);
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);
int mappages(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 size,
             uint64 perm);
uint64 walkaddr(pagetable_t pagetable, uint64 va);
pagetable_t uvmcreat(void);

// mmu.S
extern void enable_mmu(uint64 ttbr0_el1, uint64 ttbr1_el1, uint64 tcr_el1,
                       uint64 mair_el1);

// proc.c
void proc_mapstacks(pagetable_t);
void procinit(void);
int cpuid(void);
struct cpu* mycpu(void);
struct proc* myproc(void);
void userinit(void);
struct proc* newproc(void);
void yield(void);
void sleep(void* chan);
void wakeup(void* chan);
int proc_wait(void);
void proc_exit(void);
int proc_kill(int pid);
void scheduler(void);
void sched();

// trap.c
void trapinit(void);
void trapinithart(void);
void kerneltrap(int);
uint64 usertrap(int);
void prepare_return(void);
void show_invalid_entry_message(uint64, int, uint64, uint64, uint64);

// syscall.c
void syscall(void);
int sys_fork(void);

// exec.c
void exec(struct proc* p);

// swtch.S
struct context;
void swtch(struct context*, struct context*);

// timer.c
void timerinit(void);

// pipe.c
struct pipe;
struct fd;
int pipe_alloc(struct fd**, struct fd**);
int pipe_write(struct pipe* p, uint64 addr, int n);
int pipe_read(struct pipe* p, uint64 buf, int n);
void pipe_close(struct pipe* p, int writeopen);

// fd.c
int fd_ualloc(void);
struct fd* fd_alloc(void);
int fd_write(struct fd* fd, uint64 addr, int n);
int fd_read(struct fd* fd, uint64 buf, int n);
void fd_close(struct fd* fd);
void fd_incref(struct fd* fd);

uint64 get_entry(void);

// spinlock.c
void acquire(struct spinlock* lk);
void release(struct spinlock* lk);

// sdchi.c
int sd_init(void);
int sd_readblock(unsigned int lba, unsigned char* buffer, unsigned int num);
int sd_writeblock(unsigned char* buffer, unsigned int lba, unsigned int num);

// bio.c
struct buf* bread(uint sector);

// fs.c
struct inode* iget(uint inum);
