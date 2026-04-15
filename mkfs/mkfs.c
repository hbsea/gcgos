#include "../include/types.h"
#include "../include/fs.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int nblocks = 2011;
int ninodes = 100;
int size = 2048;

char zero[512] = {0};
int fsfd;
struct supperblock sb;
uint freeblock;
uint usedblocks;
uint bitblocks;
uint freeinode = 1;

void wsect(uint sec, void* buf)
{
    if (lseek(fsfd, sec * 512, 0) != sec * 512)
    {
        printf("w_lseek error\n");
        exit(1);
    }
    if (write(fsfd, buf, 512) != 512)
    {
        printf("write error\n");
        exit(1);
    };
}
void rsect(uint sec, void* buf)
{
    if (lseek(fsfd, sec * 512, 0) != sec * 512)
    {
        printf("r_lseek error\n");
        exit(1);
    }
    if (read(fsfd, buf, 512) != 512)
    {
        printf("read error\n");
        exit(1);
    }
}
uint i2b(uint inum) { return (inum / IPB) + 2; }
void winode(uint inum, struct dinode* ip)
{
    char buf[512];
    struct dinode* dip;
    uint block_num = i2b(inum);
    rsect(block_num, buf);
    dip = ((struct dinode*)buf) + (inum % IPB);
    *dip = *ip;
    wsect(block_num, buf);
}
void rinode(uint inum, struct dinode* ip)
{
    char buf[512];
    struct dinode* dip;
    uint block_num = i2b(inum);
    rsect(block_num, buf);
    dip = ((struct dinode*)buf) + (inum % IPB);
    *ip = *dip;
}
uint ialloc(short type)
{
    uint inum = freeinode++;
    struct dinode din;

    bzero(&din, sizeof(din));
    din.type = type;
    din.nlink = 1;
    din.size = 0;
    winode(inum, &din);
    return inum;
}
void balloc(int used)
{
    char buf[512];
    printf("balloc: first %d blocks have been allocated\n", used);
    bzero(buf, sizeof(buf));
    for (int i = 0; i < used; i++)
    {
        buf[i / 8] = buf[i / 8] | (0x1 << (i % 8));
    }
    printf("balloc: write bitmap block at sector %d\n", ninodes / IPB + 3);
    wsect(ninodes / IPB + 3, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))
void iappend(uint inum, void* xp, int n)
{
    char* p = xp;
    uint fbn, off, n1, wbn;
    char buf[512], indirect_buf[512];
    struct dinode din;

    rinode(inum, &din);

    off = din.size;
    while (n > 0)
    {
        fbn = off / 512;
        if (fbn < NDIRECT)
        {
            if (din.addrs[fbn] == 0)
            {
                din.addrs[fbn] = freeblock++;
                usedblocks++;
            }
            wbn = din.addrs[fbn];
        }
        else
        {
            assert((fbn - NDIRECT) < NINDIRECT);
            if (din.addrs[NDIRECT] == 0)
            {
                din.addrs[NDIRECT] = freeblock++;
                usedblocks++;
            }
            rsect(din.addrs[NDIRECT], indirect_buf);
            uint* inp = (uint*)indirect_buf;
            if (inp[fbn - NDIRECT] == 0)
            {
                inp[fbn - NDIRECT] = freeblock++;
                usedblocks++;
            }
            wbn = inp[fbn - NDIRECT];
            wsect(din.addrs[NDIRECT], indirect_buf);
        }
        n1 = min(n, (fbn + 1) * 512 - off);
        rsect(wbn, buf);
        bcopy(p, buf + off - (fbn * 512), n1);
        wsect(wbn, buf);
        n -= n1;
        off += n1;
        p += n1;
    }
    din.size = off;

    winode(inum, &din);
}
int main(int argc, char* argv[])
{
    int rootino, inum, cc, fd;
    struct dinode din;
    struct dirent de;
    char buf[512];
    printf("fs making\n");
    fsfd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);

    printf("fsfd: %d\n", fsfd);
    // Bug when
    // IPB>ninodes,supperblock=block1,inodeblock>=block2,datablock>=block3;
    // (ninodes + IPB - 1) / IPB

    bitblocks = sb.size / (512 * 8) + 1;
    usedblocks = ninodes / IPB + 3 + bitblocks;
    freeblock = usedblocks;

    printf("start freeblock:%d\n", freeblock);
    printf("usedblocks::%d\n", usedblocks);
    printf("size::%d\n", nblocks + usedblocks);
    assert(nblocks + usedblocks == size);
    for (int i = 0; i < nblocks + usedblocks; i++) wsect(i, zero);

    // supperblock
    sb.size = size;
    sb.nblocks = nblocks;
    sb.ninodes = ninodes;
    wsect(1, &sb);

    rootino = ialloc(T_DIR);
    assert(rootino == 1);

    bzero(&de, sizeof(de));
    de.inum = rootino;
    strcpy(de.name, ".");
    iappend(rootino, &de, sizeof(de));

    bzero(&de, sizeof(de));
    de.inum = rootino;
    strcpy(de.name, "..");
    iappend(rootino, &de, sizeof(de));

    // rest of the file
    for (int i = 2; i < argc; i++)
    {
        if ((fd = open(argv[i], 0)) < 0) perror(argv[i]);

        inum = ialloc(T_FILE);

        bzero(&de, sizeof(de));
        de.inum = inum;
        char* filename = strrchr(argv[i], '/') + 1;
        printf("add fileName:%s,inum=%d\n", filename, inum);
        strcpy(de.name, filename);
        iappend(rootino, &de, sizeof(de));

        while ((cc = read(fd, buf, sizeof(buf))) > 0) iappend(inum, buf, cc);
        close(fd);
    }
    balloc(usedblocks);
    return 0;
}
