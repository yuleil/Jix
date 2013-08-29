#ifndef CACHE_H
#define CACHE_H


struct buf {
  int flags;
  int dev;
  u32 sector;
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf *qnext; // disk  request queue
  char data[512];
};

#define B_BUSY  0x1  // locked
#define B_VALID 0x2  // has been read from disk
#define B_DIRTY 0x4  //



#endif // CACHE_H
