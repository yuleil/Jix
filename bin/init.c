#include "usr.h"
#include "stdio.h"

int
main(int argc, char **argv)
{
    int  fd, pid;

    chdir("/");
    if ((fd = open("tty", O_RDWR)) < 0) {
        mknod("tty", 1, 1);
        fd = open("tty", O_RDWR);
    }
    dup(fd);
    dup(fd);

    cls();


    if ((pid = fork()) < 0) {
        fprintf(stderr, "init: fork failed\n");
        exit(-1);
    }
    if (pid == 0) {
        exec("sh", argv);
        fprintf(stderr, "init: exec sh failed\n");
        exit(-1);
    }
    while (1)
        wait(NULL, 0);

    return 0;
}
