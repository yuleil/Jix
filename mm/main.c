#include "global.h"
#include "def.h"
#include "msg.h"


int do_exec(char *, char *);
int do_fork(void);
void do_exit(int);
int do_wait(int *, int nohang);
int do_sbrk(u32);

void
task_mm(void) // 2
{
    kputs("task_mm running...");
    struct msg m;

    while (1) {
        sys_msg(RECV, &m, ANY);
        mm_current = ptable + m.source;

        switch (m.type) {
            case MSG_EXEC:
                if ((m.i1 = do_exec(m.p1, m.p2)) == 0)
                        continue;
                break;
            case MSG_FORK:
                m.i1 = do_fork();
                break;
            case MSG_EXIT:
                do_exit(m.i1);
                continue;
            case MSG_WAIT:
                m.i1 = do_wait(&m.i2, m.i2);
                break;
            case MSG_SBRK:
                m.i1 = do_sbrk(m.u1);
                break;
            default:
                m.i1 = -1;
                break;
        }


        m.type = MSG_SYSRET;
        if (m.i1 == -2) {
            m.type = MSG_BESLEEP;
            m.p1 = mm_sleep;
        }

        sys_msg(SEND, &m, m.source);
    }
}


