struct supperblock
{
    int nblocks;
    int ninodes;
};

#define NDIRECT 12
#define NINDIRECT (512 / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)
#define DIRSIZ 14
struct dinode
{
    short type;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
};

#define T_DIR 1
#define T_FILE 2

#define IPB (512 / sizeof(struct dinode))

struct dirent
{
    uint16 inum;
    char name[DIRSIZ];
};

#define NINODE 100
struct inode
{
    uint dev;
    uint inum;
    int count;
    int busy;
    short type;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
};
