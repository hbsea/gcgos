#include "defs.h"
#include "bio.h"
#include "fs.h"
#include "spinlock.h"
struct inode inode[NINODE];

struct spinlock inode_table_lock;

uint rootdev = 1;

struct inode* iget(uint inum)
{
    struct inode *ip, *nip = 0;
    struct dinode* dip;
    struct buf* bp;

    acquire(&inode_table_lock);

loop:
    for (ip = &inode[0]; ip < &inode[NINODE]; ip++)
    {
        if (ip->count > 0 && ip->inum == inum)
        {
            if (ip->busy)
            {
                sleep(ip);
                goto loop;
            }
            ip->count++;
            release(&inode_table_lock);
            return ip;
        }
        if (nip == 0 && ip->count == 0) nip = ip;
    }

    if (nip == 0) panic("out of inode");
    nip->inum = inum;
    nip->count = 1;
    nip->busy = 1;
    release(&inode_table_lock);

    bp = bread((inum / IPB) + 2);
    dip = &((struct dinode*)(bp->data))[inum % IPB];

    nip->type = dip->type;
    nip->nlink = dip->nlink;
    nip->size = dip->size;
    for (int i = 0; i <= NDIRECT; i++) nip->addrs[i] = dip->addrs[i];

    bp = bread(nip->addrs[0]);

    brelse(bp);
    return nip;
}

void ilock(struct inode* ip)
{
    acquire(&inode_table_lock);
    while (ip->busy) sleep(ip);
    ip->busy = 1;
    release(&inode_table_lock);
}
void iunlock(struct inode* ip)
{
    acquire(&inode_table_lock);
    ip->busy = 0;
    wakeup(ip);
    release(&inode_table_lock);
}

void iput(struct inode* ip)
{
    acquire(&inode_table_lock);
    ip->count--;
    ip->busy = 0;
    wakeup(ip);
    release(&inode_table_lock);
}

void iincref(struct inode* ip)
{
    acquire(&inode_table_lock);
    ip->count += 1;
    release(&inode_table_lock);
}

struct inode* namei(char* path)
{
    struct inode* dp;
    char* cp = path;
    uint off;
    struct buf* bp;
    struct dirent* ep;
    int i;
    unsigned ninum;
    dp = iget(rootdev);
    while (*cp == '/') cp++;
    while (1)
    {
        if (*cp == '\0') return dp;
        if (dp->type != T_DIR)
        {
            iput(dp);
            return 0;
        }
        for (off = 0; off < dp->size; off += 512)
        {
            bp = bread(dp->addrs[off / 512]);
            for (ep = (struct dirent*)bp->data;
                 ep < (struct dirent*)bp->data + 512; ep++)
            {
                if (ep->inum == 0) continue;
                for (i = 0; i < DIRSIZ && cp[i] != '/' && cp[i]; i++)
                    if (cp[i] != ep->name[i]) break;
                if ((cp[i] == '\0' || cp[i] == '/') &&
                    (i >= DIRSIZ || ep->name[i] == '\0'))
                {
                    ninum = ep->inum;
                    brelse(bp);
                    cp += i;
                    goto found;
                }
            }
            brelse(bp);
        }
        iput(dp);
        return 0;
    found:
        iput(dp);
        dp = iget(ninum);
        while (*cp == '/') cp++;
    }
}
