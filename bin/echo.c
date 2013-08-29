#include "stdio.h"
#include "usr.h"

int
main()
{
    int c;

    ttymode(0);
    while ((c = getchar()) != EOF)
        printf("%d %c", c, 1);
    ttymode(TTY_LINE | TTY_ECHO);

    exit(0);
    return 0;
}
