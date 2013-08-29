#include "types.h"
#include "mmu.h"
#include "const.h"
#include "memlayout.h"
#include "proc.h"
#include "fs.h"

pde_t *kpgdir;

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
    [0] = 0 + PTE_P + PTE_W + PTE_PS,
    [KBASE >> PDXSHIFT] = 0 + PTE_P + PTE_W + PTE_PS,
};


struct taskstate tss = {
    .ss0 = SEG_KDATA << 3,
    .iomb = sizeof(struct taskstate),
};

struct segdesc gdt[NSEGS] = {
    [SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0),
    [SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0),
    [SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER),
    [SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER),

    //[SEG_TSS]   = SEGTSS(STS_T32A, &tss, sizeof(tss)-1, 0),
};

struct proc ptable[NPROC];

struct proc *current;

u32 nextpid;
u32 ticks;
struct context *scheduler;
struct inode inodes[NINODE];
struct file files[NFILE];
void *fs_sleep;
void *mm_sleep;
struct proc *fs_current;
struct proc *mm_current;
struct proc *initproc;
