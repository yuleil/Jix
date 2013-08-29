#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSZ  512

void readseg(void *, u32, u32);

void
bootmain()  //read elf image and jmp to kernel/entry.S::_start()
{
    struct elfhdr   *elf;
    struct proghdr  *ph, *phe;
    void *p;

    elf = (struct elfhdr *)0x10000;
    readseg(elf, 4096, 0);

    if (elf->magic != ELF_MAGIC)
        return;     //bad format

    ph = (struct proghdr *)((u8 *)elf + elf->phoff);
    for (phe = ph + elf->phnum; ph < phe; ph++) {
        p = (void *)ph->paddr;
        readseg(p, ph->filesz, ph->off);
        if (ph->memsz > ph->filesz)
            stosb((u8 *)p + ph->filesz, 0, ph->memsz - ph->filesz);
    }

    ((void(*)(void))elf->entry)();
}

void
waitdisk()
{
    while ((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

void
readsect(void *dst, u32 offset)
{
    waitdisk();
    outb(0x1F2, 1); //count
    outb(0x1F3, offset);
    outb(0x1F4, offset >> 8);
    outb(0x1F5, offset >> 16);
    outb(0x1F6, (offset >> 24) | 0xE0);
    outb(0x1F7, 0x20);//command read

    waitdisk();
    insl(0x1F0, dst, SECTSZ / 4);
}

void
readseg(void *pa, u32 count, u32 offset)
{
    u8 *p, *ep;
    p = pa;
    ep = p + count;

    p -= offset % SECTSZ;
    offset = offset / SECTSZ + 1;// kernel start from sector 1.

    for ( ; p < ep; p += SECTSZ, offset++)
        readsect(p, offset);
}
