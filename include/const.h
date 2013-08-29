#ifndef CONST_H
#define CONST_H

#define NPROC       64      // maximum number of processes
#define KSTACKSZ    4096    // size of kernel stack
#define NOFILE      16      // open files per process
#define NFILE       100     // open files per system
#define NBUF        64      // size of disk block cache
#define NINODE      64      // maximum number of active i-nodes
#define ROOTDEV     0       // device number of file system root disk
#define MAXARG      16      // max exec arguments

#define NSEGS       7

#define TASK_IDLE   0
#define TASK_SYS    1
#define TASK_MM     2
#define TASK_FS     3

#endif // CONST_H
