#ifndef GLOBAL_H
#define GLOBAL_H
#include "types.h"
#include "const.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "fs.h"


extern pde_t *kpgdir;
extern pde_t etrypgdir[];

extern struct segdesc gdt[];
extern struct taskstate tss;
extern struct proc ptable[];
extern u16 nextpid;
extern struct proc *current;

extern u32 ticks;
extern struct context *scheduler;
extern struct inode inodes[];
extern struct file files[];
extern void *fs_sleep;
extern void *mm_sleep;
extern struct proc *fs_current;
extern struct proc *mm_current;
extern struct proc *initproc;
#endif // GLOBAL_H
