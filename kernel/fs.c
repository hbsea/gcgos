#include "defs.h"
#include "bio.h"
#include "fs.h"
#include "spinlock.h"
#include "dev.h"

struct inode inode[NINODE];

struct spinlock inode_table_lock;

uint rootdev = 1;

static uint balloc(uint dev)
{
    struct buf* bp;
    struct supperblock* sb;
    int b, bi, size, ninodes;
    uchar m;

    bp = bread(dev, 1);
    sb = (struct supperblock*)bp->data;
    size = sb->size;
    ninodes = sb->ninodes;
    for (b = 0; b < size; b++)
    {
        if (b % BPB == 0)
        {
            brelse(bp);
            bp = bread(dev, BBLOCK(b, ninodes));
        }
        bi = b % BPB;
        m = 0x1 << (bi % 8);
        if ((bp->data[bi / 8] & m) == 0) break;  // is blck free?
    }
    if (b >= size) panic("balloc: out of blocks\n");
    bp->data[bi / 8] |= 0x1 << (bi % 8);
    bwrite(dev, bp, BBLOCK(b, ninodes));  // mark it allocated on disk
    return b;
}

struct inode* iget(uint dev, uint inum)
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
    nip->dev = dev;
    nip->inum = inum;
    nip->count = 1;
    nip->busy = 1;
    release(&inode_table_lock);

    bp = bread(dev, IBLOCK(inum));
    dip = &((struct dinode*)(bp->data))[inum % IPB];

    nip->type = dip->type;
    nip->major = dip->major;
    nip->minor = dip->minor;
    nip->nlink = dip->nlink;
    nip->size = dip->size;
    for (int i = 0; i <= NDIRECT; i++) nip->addrs[i] = dip->addrs[i];

    // bp = bread(dev, nip->addrs[0]);

    brelse(bp);
    return nip;
}

struct inode* ialloc(uint dev, short type)
{
    int inum;
    struct dinode* dip;
    struct buf* bp = bread(dev, 1);
    struct supperblock* sb = (struct supperblock*)bp->data;
    int ninodes = sb->ninodes;
    brelse(bp);
    for (inum = 1; inum < ninodes; inum++)
    {
        bp = bread(dev, IBLOCK(inum));
        dip = &((struct dinode*)(bp->data))[inum % IPB];
        if (dip->type == 0) break;
        brelse(bp);
    }
    dip->type = type;
    bwrite(dev, bp, IBLOCK(inum));
    brelse(bp);
    struct inode* ip = iget(dev, inum);

    return ip;
}
void iupdate(struct inode* ip)
{
    struct buf* bp;
    struct dinode* dip;
    bp = bread(ip->dev, IBLOCK(ip->inum));
    dip = &((struct dinode*)(bp->data))[ip->inum % IPB];
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;
    bwrite(ip->dev, bp, IBLOCK(ip->inum));
    brelse(bp);
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
uint bmap(struct inode* ip, uint bn)
{
    uint x;
    if (bn < NDIRECT)
        x = ip->addrs[bn];
    else
    {
        uint* inp = (uint*)bread(ip->dev, ip->addrs[NDIRECT]);
        x = inp[bn - NDIRECT];
    }
    return x;
}

#define min(a, b) ((a) < (b) ? (a) : (b))
int readi(struct inode* ip, void* xdist, uint off, uint n)
{
    struct buf* bp;
    uint target = n, n1;

    char* dst = xdist;
    while (n > 0 && off < ip->size)
    {
        bp = bread(ip->dev, bmap(ip, off / 512));
        n1 = min(n, ip->size - off);
        n1 = min(n1, 512 - (off % 512));
        char* s = (char*)bp->data + (off % 512);
        for (int i = 0; i < n1; i++) *dst++ = *s++;
        n -= n1;
        off += n1;
        dst += n1;
        brelse(bp);
    }
    return target - n;
}
int writei(struct inode* ip, void* addr, uint n)
{
    if (ip->type == T_DEV)
    {
        return devsw[ip->major].d_write(ip->minor, addr, n);
    }
    else
    {
        panic("writei: unkown type\n");
        return -1;
    }
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
    dp = iget(rootdev, 1);
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
            bp = bread(dp->dev, bmap(dp, off / 512));
            for (ep = (struct dirent*)bp->data;
                 ep < (struct dirent*)(bp->data + 512); ep++)
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
        dp = iget(dp->dev, ninum);
        while (*cp == '/') cp++;
    }
}

int mknod(char* path, short type, short major, short minor)
{
    struct dirent* ep;
    struct buf* bp;
    int off;
    struct inode* dp = iget(rootdev, 1);
    struct inode* ip = ialloc(dp->dev, type);
    ip->major = major;
    ip->minor = minor;
    ip->size = 0;
    ip->nlink = 0;
    // ip is in memeroy,iupdate wirite it on disk;
    iupdate(ip);  // write new inode to disk
    for (off = 0; off < dp->size; off += 512)
    {
        bp = bread(dp->dev, bmap(dp, off / 512));
        for (ep = (struct dirent*)bp->data;
             ep < (struct dirent*)(bp->data + 512); ep++)
        {
            if (ep->inum == 0)
            {
                goto found;
            }
        }
        brelse(bp);
    }
    panic("mknod:no dir entry free\n");
found:
    ep->inum = ip->inum;
    for (int i = 0; i < DIRSIZ && path[i]; i++) ep->name[i] = path[i];
    bwrite(dp->dev, bp, bmap(dp, off / 512));  // write directory block
    brelse(bp);

    dp->size += sizeof(struct dirent);  // update directory inode
    iupdate(dp);
    iput(dp);
    iput(ip);
    return 0;
}
