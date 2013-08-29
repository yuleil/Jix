#include "usr.h"

extern int main(int, char **);

void
_start(int argc, char **argv)  // ld excepted this symbol
{
    exit(main(argc, argv));
}
