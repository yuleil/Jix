#include "msg.h"
#include "def.h"
#include "const.h"

int
exec(char *path, char *argv[])
{
    int i;
    char buf[1024], *p;
    struct msg m;


    for (p = buf, i = 0; argv[i]; i++) {
        strcpy(p, argv[i]);
        p += strlen(argv[i]) + 1;
    }
    *p = 0;
    m.p1 = path;
    m.p2 = buf;
    m.i1 = p - buf + 1;
    m.type = MSG_EXEC;

    assert(sys_msg(BOTH, &m, TASK_MM) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
fork(void)
{
    struct msg m;

    m.type = MSG_FORK;
    assert(sys_msg(BOTH, &m, TASK_MM) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

void
exit(int ret)
{
    struct msg m;
    m.type = MSG_EXIT;
    m.i1 = ret;
    sys_msg(BOTH, &m, TASK_MM);
    // never return
}

int
wait(int *state, int nohang)
{
    struct msg m;

    do {
        m.type = MSG_WAIT;
        m.i2 = nohang;
        assert(sys_msg(BOTH, &m, TASK_MM) == 0);
    } while (m.type == MSG_BESLEEP);
    assert(m.type == MSG_SYSRET);

    if (state)
        *state = m.i2;

    return m.i1;
}

int
sbrk(u32 n)
{
    struct msg m;

    m.type = MSG_SBRK;
    m.u1 = n;
    assert(sys_msg(BOTH, &m, TASK_MM) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}
