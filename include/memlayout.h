#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#define EXTMEN  0x100000
#define PHYSTOP 0x0E000000
#define DEVSPACE 0xFE000000

#define KBASE   0x80000000
#define KLINK   (KBASE + EXTMEN)

#include "types.h"
#define V2P(a) ((u32)a - KBASE)
#define P2V(a) ((u32)a + KBASE)
#endif // MEMLAYOUT_H
