#include "msg.h"
#include "global.h"
#include "const.h"
#include "proc.h"
#include "def.h"

int recvmsg(struct msg *m, int source);
int sendmsg(struct msg *m, int dest);


void mcopyin(void *iobuf, struct msg *mbuf, struct msg *m);
void mcopyout(struct msg *mbuf, struct msg *m);

#define BUFLEN 1024

int
sendrecv(int function, struct msg *m, int dest)
{
    struct msg mbuf;
    char iobuf[BUFLEN];
    int ret;
    
    ret = -1;

    assert((dest >= 0 && dest < NPROC) ||
           (function == RECV && dest == ANY));
    assert(dest == ANY || ptable[dest].state != UNUSED);

    m->source = current - ptable;
    assert(m->source != dest);
     // mbuf is in kernel address space;

    if (function == SEND) {

        mbuf = *m;
        ret = sendmsg(&mbuf, dest);

    } else if (function == RECV) {

        ret = recvmsg(&mbuf, dest);
        *m = mbuf;

    } else if (function == BOTH) {

        mbuf = *m;
        mcopyin(iobuf, &mbuf, m);
        if ( (ret = sendmsg(&mbuf, dest)) != 0)
            return ret;
        ret = recvmsg(&mbuf, dest);
        mcopyout(&mbuf, m);
        *m = mbuf;

    }

    return ret;
}


void
mcopyin(void *iobuf, struct msg *mbuf, struct msg *m)
{
    int t;
    switch (m->type) {
        case MSG_WRITE:
            memcpy(iobuf, m->p1, m->u1);
            mbuf->p1 = iobuf;
            break;
        case MSG_MKDIR:
        case MSG_MKNOD:
        case MSG_UNLINK:
        case MSG_CHDIR:
        case MSG_OPEN:
            strncpy(iobuf, m->p1, BUFLEN);
            mbuf->p1 = iobuf;
            break;
        case MSG_LINK:
            strncpy(iobuf, m->p1, BUFLEN);
            t = strlen(iobuf);
            strncpy(iobuf + t + 1, m->p2, BUFLEN - t - 1);
            mbuf->p1 = iobuf;
            mbuf->p2 = iobuf + t + 1;
            break;
        case MSG_READ:
        case MSG_STAT:
            mbuf->p1 = iobuf;
            break;
        case MSG_EXEC:
            strncpy(iobuf, m->p1, BUFLEN);
            t = strlen(m->p1);
            memcpy(iobuf + t + 1, m->p2, m->i1);
            mbuf->p1 = iobuf;
            mbuf->p2 = iobuf + t + 1;
            break;

        default:
            break;

    }
}

void
mcopyout(struct msg *mbuf, struct msg *m)
{
    switch (m->type) {
        case MSG_READ:
            memcpy(m->p1, mbuf->p1,
                    mbuf->i1 > 0 ? mbuf->i1 : 0);
            break;
        case MSG_STAT:
            memcpy(m->p1, mbuf->p1, mbuf->i1 == 0 ?
                    sizeof(struct stat) : 0);
            break;
        default:
            break;
    }
}


int
sendmsg(struct msg *m, int dest)
{
    struct proc *p;
    int me;
    
    me = current - ptable;

    cli();
    for (p = ptable + dest; p->state == SENDING;
            p = ptable + p->sendto)
        if (p->sendto == me) {
            sti();
            panic("send dead lock");
            return -1;
        }

    p = ptable + dest;
     //ensure p->state didn't change
    if (p->state == RECVING &&
                (p->recvfrom == me ||
                p->recvfrom == ANY)) {
        memcpy(p->msgp, m, sizeof(*m));
        p->msgstate = 0;
        if (m->type == MSG_BESLEEP) {
            p->state = SLEEPING;
            p->tmp = m->p1;
        } else
            p->state = RUNNABLE;

        sti();
        return 0;
    }

    // wait for recv
    current->state  = SENDING;
    current->msgp   = m;
    current->sendto = dest;
    current->msgstate = -1;
    sched();
    sti();
    return current->msgstate;
}

int
recvmsg(struct msg *m, int source)
{
    struct proc *p;
    int me;
    
    me = current - ptable;

    cli();
    for (p = ptable + source; p->state == RECVING;
            p = ptable + p->recvfrom)
        if (p->recvfrom == me) {
            sti();
            panic("recv dead lock");
            return -1;
        }


    if (source == ANY) {
        for (p = ptable; p < ptable + NPROC; p++)
            if (p->state == SENDING &&
                    p->sendto == me)
                break;
    } else {
        p = ptable + source;
        if (!(p->state == SENDING && p->sendto == me))
            p = ptable + NPROC;
    }

    if (p == ptable + NPROC) { // didn't find
        current->state = RECVING;
        current->msgp = m;
        current->recvfrom = source;
        current->msgstate = -1;
        sched();
        sti();
        return current->msgstate;
    } else {
        memcpy(m, p->msgp, sizeof(*m));
        p->msgstate = 0;

        if (m->type == MSG_BESLEEP) { // a special msg
            p->state = SLEEPING;
            p->tmp = m->p1;
            sched();
        } else
            p->state = RUNNABLE;
            
        sti();
        return 0;
    }
}
