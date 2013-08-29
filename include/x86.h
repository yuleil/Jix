#ifndef X86_H
#define X86_H

#include "types.h"

static inline u8
inb(u16 port)
{
    u8 data;
    __asm__ __volatile__("in %1, %0" :
                         "=a" (data) :
                         "d"  (port));
    return data;
}

static inline void
insl(u16 port, void *addr, int cnt)
{
    __asm__ __volatile__("cld; rep insl" :
                         "=D" (addr), "=c" (cnt) :
                         "d" (port), "0" (addr), "1" (cnt) :
                         "memory", "cc");
}

static inline void
outb(u16 port, u8 data)
{
    __asm__ __volatile__("out %0, %1" : :
                         "a" (data), "d" (port));
}

static inline void
outw(u16 port, u16 data)
{
    __asm__ __volatile__("out %0,%1" : :
                         "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt)
{
    __asm__ __volatile__("cld; rep outsl" :
                         "=S" (addr), "=c" (cnt) :
                         "d" (port), "0" (addr), "1" (cnt) :
                         "cc");
}

static inline void
stosb(void *addr, int data, int cnt)
{
    __asm__ __volatile__("cld; rep stosb" :
                         "=D" (addr), "=c" (cnt) :
                         "0" (addr), "1" (cnt), "a" (data) :
                         "memory", "cc");
}


static inline void
stosl(void *addr, int data, int cnt)
{
    __asm__ __volatile__("cld; rep stosl" :
                         "=D" (addr), "=c" (cnt) :
                         "0" (addr), "1" (cnt), "a" (data) :
                         "memory", "cc");
}

struct segdesc;

static inline void
lgdt(struct segdesc *p, int size)
{
    volatile u16 pd[3];

    pd[0] = size-1;
    pd[1] = (u32)p;
    pd[2] = (u32)p >> 16;

    asm volatile("lgdt (%0)" : : "r" (pd));
}

struct gatedesc;

static inline void
lidt(struct gatedesc *p, int size)
{
    volatile u16 pd[3];

    pd[0] = size-1;
    pd[1] = (u32)p;
    pd[2] = (u32)p >> 16;

    asm volatile("lidt (%0)" : : "r" (pd));
}

static inline void
ltr(u16 sel)
{
    asm volatile("ltr %0" : : "r" (sel));
}

static inline u32
readeflags(void)
{
    u32 eflags;
    asm volatile("pushfl; popl %0" : "=r" (eflags));
    return eflags;
}

static inline void
loadgs(u16 v)
{
    asm volatile("movw %0, %%gs" : : "r" (v));
}

static inline void
cli(void)
{
    asm volatile("cli");
}

static inline void
sti(void)
{
    asm volatile("sti");
}

static inline u32
xchg(volatile u32 *addr, u32 newval)
{
    u32 result;

    // The + in "+m" denotes a read-modify-write operand.
    asm volatile("lock; xchgl %0, %1" :
                 "+m" (*addr), "=a" (result) :
                 "1" (newval) :
                 "cc");
    return result;
}

static inline u32
rcr2(void)
{
    u32 val;
    asm volatile("movl %%cr2,%0" : "=r" (val));
    return val;
}

static inline void
lcr3(u32 val)
{
    asm volatile("movl %0,%%cr3" : : "r" (val));
}


// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
__attribute__((packed)) struct trapframe {


    // rest of trap frame
    u16 gs;
    u16 padding1;
    u16 fs;
    u16 padding2;
    u16 es;
    u16 padding3;
    u16 ds;
    u16 padding4;

    // registers as pushed by pusha
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 oesp;      // useless & ignored
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 trapno;

    // below here defined by x86 hardware
    u32 err;
    u32 eip;
    u16 cs;
    u16 padding5;
    u32 eflags;

    // below here only when crossing rings
    u32 esp;
    u16 ss;
    u16 padding6;
};




#endif // X86_H
