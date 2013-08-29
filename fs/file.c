#include "types.h"
#include "fs.h"
#include "const.h"
#include "global.h"
#include "def.h"


struct file *
filealloc(void)
{
    struct file *f;
    for (f = files; f < files + NFILE; f++) {
        if (f->ref == 0) {
            f->ref = 1;
            return f;
        }
    }
    return NULL;
}


struct file *
filedup(struct file *f)
{
    assert(f->ref > 0);
    f->ref++;

    return f;
}

void
fileclose(struct file *f)
{
    assert(f->ref > 0);
    if (--f->ref > 0)
        return;

    if (f->type == FD_PIPE)
        pipeclose(f->pipe, f->writable);
    else if (f->type == FD_INODE) {
        iput(f->ip);
    }

    f->ref = 0;
    f->type = FD_NONE;
}


int
filestat(struct file *f, struct stat *st)
{
    if (f->type == FD_INODE) {
        stati(f->ip, st);
        return 0;
    }
    return -1;
}


int
fileread(struct file *f, void *addr, u32 n)
{
    int r;

    if (!f->readable) {
        if (f->type == FD_PIPE) kputs("read a unreadable pipe");
        return -1;
    }
    if (f->type == FD_PIPE)
        return piperead(f->pipe, addr, n);
    if (f->type == FD_INODE) {
        if((r = readi(f->ip, addr, f->off, n)) > 0)
            f->off += r;
        return r;
    }
    panic("fileread");
    return -1;
}


int
filewrite(struct file *f, void *addr, u32 n)
{
    int r;

    if(!f->writable)
        return -1;
    if(f->type == FD_PIPE)
        return pipewrite(f->pipe, addr, n);
    if(f->type == FD_INODE){
            if ((r = writei(f->ip, addr, f->off, n)) > 0)
                f->off += r;
        return r == n ? n : -1;
    }
    panic("filewrite");
    return -1;
}

