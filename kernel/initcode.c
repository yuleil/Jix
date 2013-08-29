#include "msg.h"
#include "def.h"
#include "const.h"

int
exec(char *path, char *argv[]);

int
main()
{
    char *argv[] = {"init",
                    0};
    exec("/init", argv);
    while (1)
        ;
}

int
nstrcpy(char *s, const char *t)
{
    int n;

    for (n = 0; (s[n] = t[n]) != '\0'; n++)
        ;

    return n;
}

int
exec(char *path, char *argv[])
{
    int i;
    char buf[400], *p;
    struct msg m;


    for (p = buf, i = 0; argv[i]; i++)
        p += nstrcpy(p, argv[i]) + 1;

    *p = 0;
    m.p1 = path;
    m.p2 = buf;
    m.i1 = p - buf + 1; // copy cnt
    m.type = MSG_EXEC;

    sys_msg(BOTH, &m, TASK_MM);

    return 0;
}


