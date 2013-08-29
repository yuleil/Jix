#include "types.h"
#include "cache.h"
#include "const.h"
#include "def.h"


struct {
    struct buf buf[NBUF];
    struct buf head;  //head.prev point to the last in list
} cache;

void
binit()
{
    struct buf *b;
    cache.head.next = &cache.head;
    for (b = cache.buf; b < cache.buf + NBUF; b++) {
        b->next = cache.head.next;
        b->prev = &cache.head;
        b->dev = -1;
        cache.head.next->prev = b;
        cache.head.next = b;
    }
    cache.head.prev = cache.buf;  //the last cache block
}




static struct buf *
bget(u32 dev, u32 sector)
{
    struct buf *b;

  // Try for cached block.
    for (b = cache.head.next; b != &cache.head; b = b->next)
        if (b->dev == dev && b->sector == sector)  //found
                return b;

  // Victim the tail
    b = cache.head.prev;
    b->dev = dev;
    b->sector = sector;
    b->flags = 0;
    return b;
}

// Return a B_BUSY buf with the contents of the indicated disk sector.
struct buf*
bread(u32 dev, u32 sector)
{
    struct buf *b;

    b = bget(dev, sector);
    if(!(b->flags & B_VALID))
        iderw(b);
    return b;
}

// Write b's contents to disk.
void
bwrite(struct buf *b)
{
    b->flags |= B_DIRTY;
    iderw(b);
}

// Release the buffer b.
void
brelse(struct buf *b)   //put in the head
{
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = cache.head.next;
    b->prev = &cache.head;
    cache.head.next->prev = b;
    cache.head.next = b;
}





