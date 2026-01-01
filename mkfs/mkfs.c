#include "../include/types.h"
#include "../include/fs.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int nblocks = 49;
int ninodes = 100;
char zero[512] = {0};
int fd;
uint freeblock;
struct supperblock sb;
void wsect(uint sec, void* buf)
{
    if (lseek(fd, sec * 512, 0) != sec * 512)
    {
        printf("w_lseek error\n");
        exit(1);
    }
    if (write(fd, buf, 512) != 512)
    {
        printf("write error\n");
        exit(1);
    };
}
void rsect(uint sec, void* buf)
{
    if (lseek(fd, sec * 512, 0) != sec * 512)
    {
        printf("r_lseek error\n");
        exit(1);
    }
    if (read(fd, buf, 512) != 512)
    {
        printf("read error\n");
        exit(1);
    }
}
void winode(uint inum, struct dinode* ip)
{
    char buf[512];
    struct dinode* dip;
    uint block_num = (inum / IPB) + 2;
    rsect(block_num, buf);
    dip = ((struct dinode*)buf) + (inum % IPB);
    *dip = *ip;
    printf("block_num %d offset %d\n", block_num,
           (unsigned)dip - (unsigned)buf);
    wsect(block_num, buf);
}
int main(int argc, char* argv[])
{
    int i;
    struct dinode din;
    char dbuf[512];
    printf("fs making\n");
    printf("argc :%d argv[1]:%s\n", argc, argv[1]);
    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);

    printf("fd: %d\n", fd);
    freeblock = ninodes / IPB + 2;
    printf("start freeblock:%d\n", freeblock);
    for (i = 0; i < nblocks + (ninodes / IPB) + 3; i++) wsect(i, zero);

    sb.nblocks = nblocks;
    sb.ninodes = ninodes;
    wsect(1, &sb);

    din.type = T_DIR;
    din.nlink = 2;
    din.size = 512;
    din.addrs[0] = freeblock++;
    winode(1, &din);

    ((struct dirent*)dbuf)[0].inum = 1;
    strcpy(((struct dirent*)dbuf)[0].name, ".");
    ((struct dirent*)dbuf)[1].inum = 1;
    strcpy(((struct dirent*)dbuf)[1].name, "..");
    wsect(din.addrs[0], dbuf);

    return 0;
}
