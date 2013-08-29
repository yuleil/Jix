// hd driver code.

#include "types.h"
#include "cache.h"
#include "def.h"
#include "trap.h"
#include "x86.h"

#define IDE_BSY       0x80
#define IDE_DRDY      0x40  //ready
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30

// idequeue points to the buf now being read/written to the disk.

static struct buf *idequeue;
static int nr_hd;


static int
idewait(int checkerr) //if not 0 , return error? else 0
{
    int in;

    while( ((in = inb(0x1f7)) & (IDE_BSY|IDE_DRDY))
           != IDE_DRDY)
        ;
    if(checkerr && (in & (IDE_DF|IDE_ERR)) != 0)
        return -1;
    return 0;
}

void
ideinit(void)
{
    nr_hd = *(u8 *)0x80000475;
    enable_irq(IRQ_CASCADE);
    enable_irq(IRQ_IDE);
}

// Start the request for b
static void
idestart(struct buf *b)
{
    assert(b != NULL);

    idewait(0);
    outb(0x3f6, 0);  // generate interrupt
    outb(0x1f2, 1);  // number of sectors
    outb(0x1f3, b->sector & 0xff);
    outb(0x1f4, (b->sector >> 8) & 0xff);
    outb(0x1f5, (b->sector >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((b->dev & 1) << 4) |
            ((b->sector >> 24) & 0x0f));
    if (b->flags & B_DIRTY) {
        outb(0x1f7, IDE_CMD_WRITE);
        outsl(0x1f0, b->data, 512/4);
    } else
        outb(0x1f7, IDE_CMD_READ);
}


void
ideintr(void)
{
    struct buf *b;

    if ((b = idequeue) == NULL)
    //spurious IDE interrupt
        return;

    idequeue = b->qnext;

    if (!(b->flags & B_DIRTY) && idewait(1) == 0) //ready
        insl(0x1f0, b->data, 512/4);

  // Wake process waiting for this buf.
    b->flags |= B_VALID;
    b->flags &= ~B_DIRTY;
    wake_up(b);

    if (idequeue != NULL)   //start next request
        idestart(idequeue);
}


void
iderw(struct buf *b)
{
    struct buf **pp;

    if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
        return;
    if(b->dev >= nr_hd)
        panic("iderw: unknow device");


  // Append b to idequeue.
    b->qnext = NULL;
    for(pp = &idequeue; *pp; pp = &(*pp)->qnext)
        ;
    *pp = b;

    if(idequeue == b)
        idestart(b);
    cli();
    if (idequeue == b)  // intr didn't happened
        sleep_on(b);
    sti();
}

