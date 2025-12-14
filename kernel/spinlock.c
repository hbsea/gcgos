#include "types.h"
#include "defs.h"
#include "arm.h"
#define LOCK_FREE -1

int kernel_lock = LOCK_FREE;

void acquire_spinlock(int* lock)
{
    int cpu_id = cpuid();
    // printf("acquire: %d\n", cpu_id);
    if (*lock == cpu_id)
    {
        return;
    }
    intr_off();
    // 操作需要原子性
    while (1)
    {
        if (*lock == -1)
        {
            *lock = cpu_id;
            break;
        }
    }
}

void release_spinlock(int* lock)
{
    int cpu_id = cpuid();
    // printf("release: %d\n", cpu_id);
    if (*lock != cpu_id)
        panic("release_spinlock: release a lock that i don't own\n");
    *lock = LOCK_FREE;
    intr_on();
}
