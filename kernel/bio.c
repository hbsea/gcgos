#include "defs.h"
#include "types.h"
#include "bio.h"
#include "spinlock.h"
struct buf buf[NBUF];
struct spinlock buf_talbe_lock;

struct buf* getblk()
{
    while (1)
    {
        for (int i = 0; i < NBUF; i++)
        {
            if ((buf[i].flags & B_BUSY) == 0)
            {
                buf[i].flags |= B_BUSY;
                return buf + i;
            }
        }
    }
}

struct buf* bread(uint sector)
{
    struct buf* b;
    b = getblk();
    sd_readblock(sector, b->data, 1);
    printf("%s\n", b->data);
    return b;
}
void brelse(struct buf* b) { b->flags &= ~B_BUSY; }
