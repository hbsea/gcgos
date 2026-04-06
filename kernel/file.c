#include "fs.h"
#include "defs.h"
void file_close(struct inode* ip) { iput(ip); }
