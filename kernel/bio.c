#include "defs.h"
#include "types.h"
#include "bio.h"
#include "spinlock.h"
struct buf buf[NBUF];
struct spinlock buf_talbe_lock;

struct buf* getblk()
{
    acquire(&buf_talbe_lock);
    while (1)
    {
        for (int i = 0; i < NBUF; i++)
        {
            if ((buf[i].flags & B_BUSY) == 0)
            {
                buf[i].flags |= B_BUSY;
                release(&buf_talbe_lock);
                return buf + i;
            }
        }
    }
}

// TODO:no cache
struct buf* bread(uint dev, uint sector)
{
    struct buf* b;
    b = getblk();
    sd_readblock(sector, b->data, 1);
    // printf("%s\n", b->data);
    return b;
}

void bwrite(uint dev, struct buf* b, uint sector)
{
    acquire(&buf_talbe_lock);
    sd_writeblock(b->data, sector, 1);
    release(&buf_talbe_lock);
}

void brelse(struct buf* b) { b->flags &= ~B_BUSY; }
