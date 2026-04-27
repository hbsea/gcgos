struct devsw
{
    int (*d_open)(char *, int);
    int (*d_read)(int, char *, int);
    int (*d_write)(int, void *, int);
    int (*d_close)(int);
};

extern struct devsw devsw[];
#define CONSOLE 1
