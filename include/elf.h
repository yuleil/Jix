#ifndef ELF_H
#define ELF_H

#define ELF_MAGIC 0x464c457fu //'\x7felf' little endian

#include "types.h"
struct elfhdr {
    u32 magic;
    u8  elf[12];
    u16 type;
    u16 machine;
    u32 version;
    u32 entry;
    u32 phoff;
    u32 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
};

struct proghdr {
    u32 type;
    u32 off;
    u32 vaddr;
    u32 paddr;
    u32 filesz;
    u32 memsz;
    u32 flags;
    u32 align;
};

#define ELF_PROG_LOAD       1   //proghdr.type

#define ELF_PROG_FLAG_EXEC  1   //proghdr.flag
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ  4

#endif // ELF_H
