#ifndef MSG_H
#define MSG_H
#include "types.h"

struct msg {
    int type;
    int source;
    void *p1;  //str or data
    void *p2;
    u32 u1;
    u32 u2;
    int i1;  //as ret
    int i2;
};


enum MSGTYPE {
    MSG_BAD = 1,
    MSG_SYSRET,
    MSG_BESLEEP,   //tryagain
// fs
    MSG_OPEN,
    MSG_CLOSE,
    MSG_READ,
    MSG_WRITE,
    MSG_CHDIR,
    MSG_DUP,
    MSG_LINK,
    MSG_UNLINK,
    MSG_MKDIR,
    MSG_MKNOD,
    MSG_PIPE,
    MSG_STAT,
    MSG_SEEK,
    MSG_GOTOXY,
    MSG_TTYMODE,
    MSG_CLS,
    MSG_TTYCOLOR,

// mm
    MSG_FORK,
    MSG_EXEC,
    MSG_EXIT,
    MSG_WAIT,
    MSG_SBRK,
// sys
    MSG_PID,
    MSG_PPID,
    MSG_SLEEP,
    MSG_TICKS,
};

#define ANY -1
#define SEND 1
#define RECV 2
#define BOTH 3

#endif // MSG_H
