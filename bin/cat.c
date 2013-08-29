#include "usr.h"

int
main(int argc, char **argv)
{
    int fd, n;
    char buff[512];

    fd = argc >= 2 ? open(argv[1], O_RDONLY) : 0;

    while ((n = read(fd, buff, sizeof(buff))) > 0)
        write(1, buff, n);

    return 0;
}
