#include "types.h"
#include "defs.h"
#include "bio.h"
#include "fs.h"
struct inode inode[NINODE];

struct inode* iget(uint inum)
{
    struct dinode* dip;
    struct buf* bp;
    bp = bread((inum / IPB) + 2);
    dip = &((struct dinode*)(bp->data))[inum % IPB];
    uint blk_num;
    blk_num = dip->addrs[0];
    unsigned char buf_r[512];
    sd_readblock(blk_num, buf_r, 1);
    struct dirent* dir;
    dir = (struct dirent*)buf_r;
}
