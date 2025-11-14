#include "types.h"
#include "defs.h"
#define LOCK_FREE -1

uint32 kernel_lock = LOCK_FREE;

void acquire_spinlock(uint32* lock)
{
    int cpu_id = cpuid();
    printf("acquire: %d\n", cpu_id);
    if (*lock == cpu_id)
    {
        return;
    }
    while (1)
    {
        if (*lock == LOCK_FREE) *lock = cpu_id;
    }
}

void release_spinlock(uint32* lock)
{
    int cpu_id = cpuid();
    printf("release: %d\n", cpu_id);
    if (*lock != cpu_id)
        panic("release_spinlock: release a lock that i don't own\n");
    *lock = LOCK_FREE;
}
