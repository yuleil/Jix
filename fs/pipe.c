#include "fs.h"
#include "def.h"
#include "global.h"

int
pipealloc(struct file **rf, struct file **wf)
{
    struct pipe *p;

    p = 0;
    *rf = *wf = NULL;

    if ((*rf = filealloc()) == 0 || (*wf = filealloc()) == 0)
        goto bad;
    if ((p = (struct pipe*)kalloc1()) == NULL)
        goto bad;

    p->readopen = 1;
    p->writeopen = 1;
    p->head = 0;
    p->count = 0;

    (*rf)->type = FD_PIPE;
    (*rf)->readable = 1;
    (*rf)->writable = 0;
    (*rf)->pipe = p;

    (*wf)->type = FD_PIPE;
    (*wf)->readable = 0;
    (*wf)->writable = 1;
    (*wf)->pipe = p;

    return 0;


bad:
    if(p)
        kfree1((char *)p);
    if(*rf)
        fileclose(*rf);
    if(*wf)
        fileclose(*wf);
    return -1;
}

void
pipeclose(struct pipe *p, int writable)
{
    if(writable){
        p->writeopen = 0;
        wake_up1(&p->readopen);
    } else {
        p->readopen = 0;
        wake_up1(&p->writeopen);
    }
    if (p->readopen == 0 && p->writeopen == 0)
        kfree1((char *)p);
}


int
pipewrite(struct pipe *p, char *s, u32 n)
{
    int i;


    if (!p->readopen)
        return -1;

    for (i = 0; i < n; i++) {
        if (p->count == PIPESIZE) {
            wake_up1(&p->readopen);
            if (i)
                return i;
            else {
                fs_sleep = &p->writeopen;
                return -2;
            }
        }
        p->data[(p->head + p->count++) % PIPESIZE] = s[i];
    }
    wake_up1(&p->readopen);
    return n;
}

int
piperead(struct pipe *p, char *d, u32 n)
{
    int i;


    for (i = 0; i < n; i++) {
        if  (!p->count) {
            if (!i) {
                if (!p->writeopen)
                    return 0;
                else {
                    fs_sleep = &p->readopen;
                    return -2;   //!!!!!!!!!!!!sleep it
                }
            }
            break;
        }
        d[i] = p->data[p->head];
        p->head = (p->head + 1) % PIPESIZE;
        p->count--;
    }
    wake_up1(&p->writeopen);
    return i;
}
