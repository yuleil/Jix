#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>


#include "fs.h"




int
main(int argc, char ** argv)
{
    if (argc < 2) {
        puts("Usage: mkfs imgname [file list]");
        return -1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("");
        return -1;
    }

    struct stat sbuf;
    fstat(fd, &sbuf);

    struct superblock sb;
    sb.size = sbuf.st_size / BSIZE;
    sb.ninodes = sb.size / 100;

    int inodeblocks = (sb.ninodes * sizeof(struct dinode)
            + BSIZE - 1) / BSIZE ;
    int bitmapblocks = (sb.size + BPB - 1) / BPB;
    sb.bitmapstart = ISTART + inodeblocks;
    int datastart = sb.bitmapstart + bitmapblocks;


    char *disk = mmap(NULL, sbuf.st_size, PROT_WRITE,
            MAP_SHARED, fd, 0);
    if (disk == (void *)(-1)) {
        perror("mmap");
        return -1;
    }
    // memset(disk + ISTART * BSIZE, 0, (sb.size - ISTART) * BSIZE);
    memset(disk + ISTART * BSIZE, 0, (inodeblocks + bitmapblocks) * BSIZE);
    // spuer block
    *(struct superblock *)(disk + (ISTART - 1) * BSIZE) = sb;
    // root inode
    struct dinode *inodes = (struct dinode *)(disk + ISTART * BSIZE);
    inodes[0].type = 0xff; // should never be used
    inodes[ROOTINO].type = T_DIR;
    inodes[ROOTINO].nlink = 1;
    inodes[ROOTINO].major = inodes[ROOTINO].minor = 0;
    inodes[ROOTINO].size = (argc + 1) * sizeof(struct dirent);

    int i, sect = 0;
    for (i = 0; i < (inodes[ROOTINO].size  + BSIZE - 1)/ BSIZE; i++)
        inodes[ROOTINO].addrs[i] = datastart + sect++;

    // root's . & ..
    struct dirent *dirs = (struct dirent *)(disk + inodes[ROOTINO].addrs[0] * BSIZE);
    dirs[0].inum = ROOTINO;
    strcpy(dirs[0].name, ".");
    dirs[1].inum = ROOTINO;
    strcpy(dirs[1].name, "..");



    // files.
    for (i = 2; i < argc; i++) {
        struct stat sbuf;
        int fd;
        int nsector;
        if ((fd = open(argv[i], O_RDWR)) < 0)
            return -1;
        fstat(fd, &sbuf);
        if (i % IPB == 0)
            inodes = (struct dinode *)(disk + (ISTART + i / IPB) * BSIZE);
        inodes[i % IPB].type = T_FILE;
        inodes[i % IPB].major = inodes[i].minor = 0;
        inodes[i % IPB].nlink = 1;
        inodes[i % IPB].size = sbuf.st_size;
        if ((nsector = (sbuf.st_size + BSIZE - 1) / BSIZE) > MAXFILE)
            return -1;
        if (nsector > NDIRECT)
            inodes[i % IPB].addrs[NDIRECT] = datastart + sect++;

        char *f = mmap(NULL, sbuf.st_size, PROT_WRITE,
            MAP_SHARED, fd, 0);
        memcpy(disk + (datastart + sect) * BSIZE, f, sbuf.st_size);
        munmap(f, sbuf.st_size);
        close(fd);

        int j;
        for (j = 0; j < nsector; j++)
            if (j < NDIRECT)
                inodes[i % IPB].addrs[j] = datastart + sect++;
            else
                ((u32 *)(disk + inodes[i % IPB].addrs[NDIRECT] * BSIZE))[j - NDIRECT] =
                    datastart + sect++;

        dirs[i].inum = i;
        char *s, *last;
        s = last = argv[i];
        for ( ; *s; s++)
            if (*s == '/')
                last = s;
        strncpy(dirs[i].name, last == s ? s : last + 1, sizeof(dirs[i].name));
    }
    // tty device
    if (argc % IPB == 0)
        inodes = (struct dinode *)(disk + (ISTART + argc / IPB) * BSIZE);
    inodes[argc % IPB].type = T_DEV;
    inodes[argc % IPB].nlink = 1;
    inodes[argc % IPB].major = 1;
    inodes[argc % IPB].minor = 0;
    inodes[argc % IPB].size = 0;
    dirs[argc].inum = argc;
    strcpy(dirs[argc].name, "tty");

    // bit map
    int n = datastart + sect;
    memset(disk + sb.bitmapstart * BSIZE, 0xFF, n / 8);

    for (i = 0; i < n % 8; i++)
        (disk + sb.bitmapstart * BSIZE)[n / 8] |= (1 << i);
    // write back
    msync(disk, sbuf.st_size, MS_SYNC);
    puts("OK");
    return 0;
}
