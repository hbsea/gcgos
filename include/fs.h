#pragma once
#include "types.h"

#define BSIZE 512
struct supperblock
{
    uint size;
    uint nblocks;
    uint ninodes;
};

#define NDIRECT 31
#define NINDIRECT (512 / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)
#define DIRSIZ 14
struct dinode
{
    short type;
    short major;
    short minor;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
};

#define T_DIR 1
#define T_FILE 2
#define T_DEV 3

// sector 0 is unused
// sector 1 is superblock
// inodes start at sector 2...
// bitmap
// data block
#define IPB (512 / sizeof(struct dinode))
#define IBLOCK(inum) (inum / IPB + 2)  // start of inode
#define BPB (BSIZE * 8)
#define BBLOCK(b, ninodes) ((b / BPB) + (ninodes / IPB) + 3)  // start of bitmap

struct dirent
{
    uint16 inum;
    char name[DIRSIZ];
};

#define NINODE 100
struct inode
{
    uint dev;
    short major;
    short minor;
    uint inum;
    int count;
    int busy;
    short type;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
};

#define O_CREATE 0x200
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
