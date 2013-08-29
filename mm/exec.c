#include "proc.h"
#include "global.h"
#include "elf.h"
#include "const.h"
#include "usr.h"
#include "def.h"
#include "mmu.h"
#include "memlayout.h"

void        freevm(pde_t *pgdir);
u32         allocuvm(pde_t *pgdir, u32 addr, u32 len);
int         loaduvm(pde_t *pgdir, char *addr, int fd, u32 off, u32 sz);

int
do_exec(char *path, char *args)
{
    char *s, *last;
    int fd, i, argc, len;
    u32 end, sp, ustack[3 + MAXARG + 1], pa, ret;
    struct elfhdr elf;
    struct proghdr ph;
    pde_t   *pgdir = NULL;
    pte_t   *pte;

    // mm_current->state == RECVING , couldn't be scheduled
    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;
    if (read(fd, &elf, sizeof(elf)) != sizeof(elf) ||
            elf.magic != ELF_MAGIC)
        goto bad;
    if ((pgdir = setupkvm(kalloc1)) == NULL)
        goto bad;


    for (end = i = 0; i < elf.phnum; i++) {
        if (seek(fd, elf.phoff + sizeof(ph) * i) < 0 ||
                read(fd, &ph, sizeof(ph)) != sizeof(ph))
            goto bad;
        if (ph.type != ELF_PROG_LOAD)
            continue;
        if (ph.memsz < ph.filesz)
            goto bad;

        if ((ret = allocuvm(pgdir, ph.vaddr, ph.memsz)) == 0 ||
                loaduvm(pgdir, (char *)ph.vaddr, fd, ph.off, ph.filesz) < 0)
            goto bad;

        end = max(ret, end);

    }
    close(fd);
    if (allocuvm(pgdir, KBASE - 2 * PGSIZE, 2 * PGSIZE) != KBASE)
        goto bad;       // alloc 2 pages for ustack



    // push argument strings, prepare rest of stack in ustack.

    if ((pte = walkpgdir(pgdir, (void *)(KBASE - PGSIZE), NULL)) == NULL)
        goto bad;
    pa = PTE_ADDR(*pte) + PGSIZE;   // p stack end
    sp = KBASE;                     // v stack end

    for (argc = 0, s = args; *s; argc++) {
        if (argc >= MAXARG)
            goto bad;
        len = strlen(s);
        sp = (sp - len - 1) & ~3;  // align to 4byte
        pa = (pa - len - 1) & ~3;
        memcpy((char *)P2V(pa), s, len + 1);
        ustack[3 + argc] = sp;
        s += len + 1;
    }
    ustack[3 + argc] = 0;

    ustack[2] = sp - (argc + 1) * 4;    // argv
    ustack[1] = argc;
    ustack[0] = 0xffffffff; // _start can not return.

    pa -= (3 + argc + 1/*for argv[argc] == NULL*/) * 4;
    sp -= (3 + argc + 1) * 4;
    memcpy((char *)P2V(pa), ustack, (3 + argc + 1) * 4);

    for (last = s = path; *s; s++)
        if (*s == '/')
            last = s + 1;
    safestrcpy(mm_current->name, last, sizeof(mm_current->name));

    freevm(mm_current->pgdir);
    mm_current->pgdir = pgdir;
    mm_current->brk = end;
    mm_current->tf->eip = elf.entry;
    mm_current->tf->esp = sp;


    mm_current->state = RUNNABLE;
    return 0;

bad:
    if (pgdir)
        freevm(pgdir);
    close(fd);
    return -1;
}



u32
allocuvm(pde_t *pgdir, u32 addr, u32 len)
{
    u32 end;
    char *mem;

    end = addr + len;
    if (end > KBASE)
        return 0;

    addr = PGROUNDDOWN(addr);
    for (; addr < end; addr += PGSIZE) {
        if ((mem = kalloc1()) == NULL) {
            return 0;
        }
        memset(mem, 0, PGSIZE);
        mappages(pgdir, (char *)addr, PGSIZE, V2P(mem),
                 PTE_W | PTE_U, kalloc1);
    }
    return PGROUNDUP(end);
}


int
loaduvm(pde_t *pgdir, char *addr, int fd, u32 off, u32 sz)
{
    u32 i, pa, n;
    pte_t *pte;

    if ((u32)addr % PGSIZE != 0)
        panic("loaduvm: unaligned page");

    for (i = 0; i < sz; i += PGSIZE) {
        if ((pte = walkpgdir(pgdir, addr + i, NULL)) == NULL)
            panic("loaduvm: walk");

        pa = PTE_ADDR(*pte);
        n = min(sz - i, PGSIZE);
        if (seek(fd, off + i) < 0 ||
            read(fd, (char *)P2V(pa), n) != n)
        return -1;
    }
    return 0;

}
