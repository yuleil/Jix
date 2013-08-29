#include "global.h"
#include "msg.h"
#include "def.h"
#include "const.h"

int do_seek(int fd, u32 off);
int do_write(int fd, void *dst, u32 cnt);
int do_read(int fd, void *src, u32 cnt);
int do_stat(int fd, struct stat *sp);
int do_open(const char *path, int mode);
int do_close(int fd);
int do_mkdir(char *path);
int do_mknod(char *path, short major, short minor);
int do_link(char *, char *);
int do_unlink(char *path);
int do_dup(int fd);
int do_chdir(char *path);
int do_pipe(int *fd);
int do_gotoxy(u32 x, u32 y);
int do_ttymode(u32 mode);
int do_cls(void);
int do_fsfork(struct proc *op, struct proc *np);
int do_fsexit(struct proc *p);
int do_ttycolor(int back, int front);

void
task_fs() // 3
{
    struct msg m;

    kputs("task_fs running...");
    binit();
    ideinit();
    tty_init();
    kputs("fs init OK...");

    while (1) {
        sys_msg(RECV, &m, ANY);
        fs_current = ptable + m.source;

        switch (m.type) {
            case MSG_OPEN:
                m.i1 = do_open(m.p1, m.i1);
                break;
            case MSG_CLOSE:
                m.i1 = do_close(m.i1);
                break;
            case MSG_READ:
                m.i1 = do_read(m.i1, m.p1, m.u1);
                break;
            case MSG_WRITE:
                m.i1 = do_write(m.i1, m.p1, m.u1);
                break;
            case MSG_CHDIR:
                m.i1 = do_chdir(m.p1);
                break;
            case MSG_DUP:
                m.i1 = do_dup(m.i1);
                break;
            case MSG_LINK:
                m.i1 = do_link(m.p1, m.p2);
                break;
            case MSG_UNLINK:
                m.i1 = do_unlink(m.p1);
                break;
            case MSG_MKDIR:
                m.i1 = do_mkdir(m.p1);
                break;
            case MSG_MKNOD:
                m.i1 = do_mknod(m.p1, m.i1, m.i2);
                break;
            case MSG_PIPE:
                m.i1 = do_pipe((int *)&m.u1);
                break;
            case MSG_STAT:
                m.i1 = do_stat(m.i1, m.p1);
                break;
            case MSG_SEEK:
                m.i1 = do_seek(m.i1, m.u1);
                break;
            case MSG_GOTOXY:
                m.i1 = do_gotoxy(m.u1, m.u2);
                break;
            case MSG_TTYMODE:
                m.i1 = do_ttymode(m.u1);
                break;
            case MSG_CLS:
                m.i1 = do_cls();
                break;
            case MSG_TTYCOLOR:
                m.i1 = do_ttycolor(m.i1, m.i2);
                break;
            case MSG_FORK:
                assert(m.source == TASK_MM);
                m.i1 = do_fsfork(m.p1, m.p2);
                break;
            case MSG_EXIT:
                assert(m.source == TASK_MM);
                m.i1 = do_fsexit(m.p1);
                break;

            default:
                m.i1 = -1;
                break;
        }

        m.type = MSG_SYSRET;
        if (m.i1 == -2) {
            m.type = MSG_BESLEEP;
            m.p1 = fs_sleep;
        }
        sys_msg(SEND, &m, m.source);
    }
}
