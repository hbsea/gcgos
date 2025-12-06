

#include "defs.h"
#define ELF_MAGIC 0x464c457f
#define ELF_PROG_LOAD 1

struct Elf
{
    uint32 magic;
    uint8 elf[12];
    uint16 type;
    uint16 machine;
    uint32 version;
    uint64 entry;
    uint64 phoff;  // ph for program header
    uint64 shoff;  // sh for section header
    uint32 flags;
    uint16 ehsize;  // eh for elf header
    uint16 phentsize;
    uint16 phnum;
    uint16 shentsize;
    uint16 shnum;
    uint16 shstrndx;  // 节头字符串表的索引
};

struct Proghdr
{
    uint32 type;
    uint32 flags;
    uint64 off;
    uint64 vaddr;
    uint64 paddr;
    uint64 filesz;
    uint64 memsz;
    uint64 align;
};
