#ifndef TYPES_H
#define TYPES_H

#ifndef __ASSEMBLER__
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

typedef u32 pte_t;
typedef u32 pde_t;

#define NULL ((void *)0)
#endif
#endif // TYPES_H
