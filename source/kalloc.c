#include "types.h"
#include "memlayout.h"
#include "arm.h"
#include "defs.h"

#define NCPU 4
__attribute__((aligned(16))) char stack0[4096 * NCPU];

extern char end[]; // first address after kernel.  defined by kernel.ld.
void freerange(void *pa_start, void *pa_end);

struct run
{
    struct run *next;
};
struct
{
    struct run *freelist;
} kmem;

void kinit()
{
    printf("k_end is at:%p\n", (void *)end);
    printf("PHYSTOP is at:%p\n", (void *)PHYSTOP);
    freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    printf("first pa page addr:%p \n", p);
    int i;
    for (i = 0; p + PGSIZE <= (char *)pa_end; i++, p += PGSIZE)
    {
        kfree(p);
    }
    printf("The total pa page is:%d; last page addr:%p \n", i, p);
}

void kfree(void *pa)
{
    struct run *r;
    if ((uint64)pa % PGSIZE != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    r = (struct run *)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
}

void *kalloc(void)
{
    struct run *r;
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;

    return (void *)r;
}
void countkmem(void)
{
    struct run *r;
    int i;
    for (i = 0, r = kmem.freelist; r->next != '\0'; r = r->next)
    {
        i++;
    }
    printf("kmem.freelist lenght: %d\n", i);
}