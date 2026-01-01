struct supperblock
{
    int nblocks;
    int ninodes;
};

#define NDIRECT 14
struct dinode
{
    short type;
    short nlink;
    uint size;
    uint addrs[NDIRECT];
};

#define T_DIR 1
#define T_FILE 2

#define IPB (512 / sizeof(struct dinode))

struct dirent
{
    uint16 inum;
    char name[14];
};

#define NINODE 100
struct inode
{
    uint dev;
    uint inum;
    int count;
    int busy;
    short type;
    short link;
    uint size;
    uint addrs[NDIRECT];
};
