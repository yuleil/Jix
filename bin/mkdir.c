#include "usr.h"

int
main(int argc, char **argv)
{
    return argc == 2 ?
                mkdir(argv[1]) : -1;
}

