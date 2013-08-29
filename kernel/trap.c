#include "x86.h"
#include "mmu.h"
#include "trap.h"
#include "global.h"
#include "def.h"

extern u32 vectors[];
struct gatedesc idt[256];



void
idtinit(void)
{
    int i;

    for(i = 0; i < 256; i++)
        SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
    SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
    lidt(idt, sizeof(idt));
}





void
timerinit(void)
{
    // Interrupt 100 times/sec.
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(IO_TIMER1, TIMER_DIV(100) % 256);
    outb(IO_TIMER1, TIMER_DIV(100) / 256);
}

void
intrinit(void)
{
    cli();
    enable_irq(IRQ_TIMER);
    enable_irq(IRQ_KBD);
    timerinit();
}

char *itoa(int n)
{
    static char buf[10];
    int i;
    for (i = 0; n; n /= 10)
        buf[i++] = '0' + n % 10;
    buf[i] = 0;
    char t;
    for (i--; i > n; i--, n++) {
        t = buf[i];
        buf[i] = buf[n];
        buf[n] = t;
    }
    return buf;
}

void
trap(struct trapframe *tf)
{


    switch (tf->trapno) {
    case T_SYSCALL:
        tf->eax = sendrecv((int)tf->ebx,
                (struct msg *)tf->ecx, (int)tf->edx);
        break;
    case T_IRQ0 + IRQ_TIMER:
        ticks++;
        if (current != ptable + TASK_IDLE)
            current->counter--;
        eoi();
        sched();
        break;

    case T_IRQ0 + IRQ_KBD:
        ttyintr();
        eoi();
        break;

    case T_IRQ0 + IRQ_IDE:
        ideintr();
        eoi();
        eoi_s();
        break;
    default:
        kputs(itoa(tf->trapno));
        break;
    }

}
