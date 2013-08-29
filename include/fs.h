#ifndef FS_H
#define FS_H
#include "types.h"

// Block 0 is boot sector. 1-2046 is elf kernel image (almost 1m)
// Block 2047 is super block.
// 2048 is the start of inode array

#define ROOTINO 1 // 0 means unused dir item
#define BSIZE 512

struct superblock {

    u32 size;           // Size of file system image (blocks)
    u32 ninodes;        // Number of inodes.
    u32 bitmapstart;
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(u32))
#define MAXFILE (NDIRECT + NINDIRECT)   // max blocks one inode could hold


#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Special device

// On-disk inode structure
struct dinode {
    short type;
    short nlink;
    short major;
    short minor;
    u32   size;
    u32   addrs[NDIRECT+1];
};

#define ISTART    2048

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block which containing inode i

#define IBLOCK(i)     ((i)/IPB  + ISTART)

// Bitmap bits per block
#define BPB           (BSIZE * 8)

// Block containing bit for block b

#define BBLOCK(b, bitmapstart) (bitmapstart + (b)/BPB )

// Directory is a file containing a sequence of dirent structures.
#define NAMELEN 16

struct dirent {
    u32 inum;
    char name[NAMELEN];
};



struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref; // reference count
    char readable;
    char writable;
    struct pipe *pipe;
    struct inode *ip;
    u32 off;
};

#define PIPESIZE 1024

struct pipe {
    u32 head;
    u32 count;
    char readopen;   // read fd is still open
    char writeopen;  // write fd is still open
    char safe[10];
    char data[PIPESIZE];
};

// in-core file system types

struct inode {
    u32 dev;           // Device number
    u32 inum;          // Inode number
    int ref;            // Reference count

    short type;         // copy of disk inode
    short nlink;
    short major;
    short minor;
    u32 size;
    u32 addrs[NDIRECT+1];
};

struct stat {
    short type;
    int dev;
    u32 ino;
    short nlink;
    u32 size;
};


#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200



#endif // FS_H
