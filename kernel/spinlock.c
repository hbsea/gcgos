#include "types.h"
#include "arm.h"
#include "spinlock.h"
#include "defs.h"

void acquire(struct spinlock* lk)
{
    struct cpu* cp = mycpu();
    printf("acquire cpu: %d\n", cpuid());
    intr_off();
    // 操作需要原子性
    while (__sync_lock_test_and_set(&lk->locked, 1) != 0);
    __sync_synchronize();
    lk->p = mycpu();
}

void release(struct spinlock* lk)
{
    lk->p = 0;
    printf("release cpu: %d\n", cpuid());
    __sync_synchronize();
    __sync_lock_release(&lk->locked);
    intr_on();
}
