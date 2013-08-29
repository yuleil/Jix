#ifndef PROC_H
#define PROC_H

#include "types.h"
#include "fs.h"
#include "const.h"
#include "x86.h"

enum procstate { UNUSED = 0, EMBRYO, SLEEPING, RUNNABLE, ZOMBIE, SENDING, RECVING};

struct context {
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    u32 eip;  // must be last
};

// Per-process state
struct proc {
    struct trapframe   *tf;            // Trap frame
    struct context     *context;       // swtch() this to run process
    enum procstate      state;          // Process state
    char               *kstack;        // Bottom of kernel stack for this process
    pde_t              *pgdir;         // Page table
    u32                 brk;
    u32                 priority;
    u32                 counter;
    u32                 pid;            // Process ID
    struct proc        *parent;            // Parent process
    void               *tmp;                  // If SLEEPING, sleeping for tmp
    struct file        *ofile[NOFILE];  // Open files
    struct inode       *cwd;           // Current directory
    char                name[16];               // Process name (debugging)

// msg
    struct msg         *msgp;
    int                 sendto;
    int                 recvfrom;    //only used in RECVING | SENDING state
    int                 msgstate;   //set after wait for msg
};



#endif // PROC_H
