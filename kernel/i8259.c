#include "x86.h"
#include "trap.h"

void
init_8259A()
{
    outb(INT_M_CTL,	0x11);			/* Master 8259, ICW1. */
    outb(INT_S_CTL,	0x11);			/* Slave  8259, ICW1. */
    outb(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	/* Master 8259, ICW2. master entry addr 0x20. */
    outb(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	/* Slave  8259, ICW2. slave entry addr  0x28. */
    outb(INT_M_CTLMASK,	0x4);			/* Master 8259, ICW3. */
    outb(INT_S_CTLMASK,	0x2);			/* Slave  8259, ICW3. */
    outb(INT_M_CTLMASK,	0x1);			/* Master 8259, ICW4. */
    outb(INT_S_CTLMASK,	0x1);			/* Slave  8259, ICW4. */

    outb(INT_M_CTLMASK,	0xFF);	/* Master 8259, OCW1. */
    outb(INT_S_CTLMASK,	0xFF);	/* Slave  8259, OCW1. */
}



void
enable_irq(u32 irq)
{
    if (irq > 15u)
        return;
    u8 mask = inb(irq < 8 ? INT_M_CTLMASK : INT_S_CTLMASK);
    mask &= ~(1 << (irq < 8 ? irq : (irq - 8)));
    outb(irq < 8 ? INT_M_CTLMASK : INT_S_CTLMASK, mask);
}

void
disable_irq(u32 irq)
{
    if (irq > 15u)
        return;
    u8 mask = inb(irq < 8 ? INT_M_CTLMASK : INT_S_CTLMASK);
    mask |= 1 << (irq < 8 ? irq : (irq - 8));
    outb(irq < 8 ? INT_M_CTLMASK : INT_S_CTLMASK, mask);
}

void
eoi()
{
    outb(INT_M_CTL, EOI);
}

void
eoi_s()
{
    outb(INT_S_CTL, EOI);
}
