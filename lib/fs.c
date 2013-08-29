#include "msg.h"
#include "const.h"
#include "def.h"

int
open(char *path, int mode)
{
    struct msg m;
    m.type = MSG_OPEN;
    m.p1 = path;
    m.i1 = mode;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

void
close(int fd)
{
    struct msg m;
    if (fd < 0 || fd >= NOFILE)
        return;
    m.type = MSG_CLOSE;
    m.i1 = fd;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return;
}

int
read(int fd, void *dst, u32 cnt)
{
    struct msg m;
    u32 nread;
    char *dest = dst;

    for (nread = 0; nread < cnt; dest += m.i1, nread += m.i1) {
        do {
            m.type = MSG_READ;
            m.i1 = fd;
            m.p1 = dest;
            m.u1 = min(cnt - nread, 1024);
            assert(sys_msg(BOTH, &m, TASK_FS) == 0);
        } while (m.type == MSG_BESLEEP);
        assert(m.type == MSG_SYSRET);

        if (m.i1 < 0)
            return nread ? nread : -1;

        if (m.i1 < min(cnt - nread, 1024))
            return nread + m.i1;
    }
    return nread;
}

int
write(int fd, void *src, u32 cnt)
{
    struct msg m;
    u32 nwritten;
    char *source = src;

    for (nwritten = 0; nwritten < cnt; source += m.i1, nwritten += m.i1) {
        do {
            m.type = MSG_WRITE;
            m.i1 = fd;
            m.p1 = src;
            m.u1 = min(cnt - nwritten, 1024);
            assert(sys_msg(BOTH, &m, TASK_FS) == 0);
        } while (m.type == MSG_BESLEEP);
        assert(m.type == MSG_SYSRET);

        if (m.i1 < 0)
            return nwritten ? nwritten : -1;

        if (m.i1 < min(cnt - nwritten, 1024))
            return nwritten + m.i1;
    }
    return nwritten;
}

int
chdir(char *path)
{
    struct msg m;

    m.type = MSG_CHDIR;
    m.p1 = path;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
dup(int fd)
{
    struct msg m;

    m.type = MSG_DUP;
    m.i1 = fd;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
link(char *n, char *target)
{
    struct msg m;

    m.type = MSG_LINK;
    m.p1 = n;
    m.p2 = target;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
unlink(char *path)
{
    struct msg m;

    m.type = MSG_UNLINK;
    m.p1 = path;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
mkdir(char *path)
{
    struct msg m;

    m.type = MSG_MKDIR;
    m.p1 = path;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
mknod(char *path, short major, short minor)
{
    struct msg m;

    m.type = MSG_MKNOD;
    m.p1 = path;
    m.i1 = major;
    m.i2 = minor;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

int
pipe(int *fd)
{
    struct msg m;

    m.type = MSG_PIPE;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    fd[0] = (int)m.u1;
    fd[1] = (int)m.u2;
    return m.i1;
}


struct stat;

int
fstat(int fd, struct stat *sp)
{
    struct msg m;

    m.type = MSG_STAT;
    m.i1 = fd;
    m.p1 = sp;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}


int
seek(int fd, u32 off)
{
    struct msg m;

    m.type = MSG_SEEK;
    m.i1 = fd;
    m.u1 = off;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return m.i1;
}

void
gotoxy(u32 x, u32 y)
{
    struct msg m;

    m.type = MSG_GOTOXY;
    m.u1 = x;
    m.u2 = y;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return;
}

void
ttymode(int mode)
{
    struct msg m;

    m.type = MSG_TTYMODE;
    m.u1 = mode;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return;
}

void
cls(void)
{
    struct msg m;

    m.type = MSG_CLS;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return;
}

void
ttycolor(int back, int front)
{
    struct msg m;
    m.type = MSG_TTYCOLOR;
    m.i1 = back;
    m.i2 = front;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);

    return;
}
