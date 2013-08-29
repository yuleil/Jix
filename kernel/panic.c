#include "types.h"

static int lineno = 0;
#define VBASE ((u16 *)0x800b8000u)

void
kputs(const char *s)
{
    int i = lineno * 80;

    do
        VBASE[i++] = (0xC << 8) | *s;
    while (*++s);

    if (lineno++ > 25)
        lineno = 0;
}

void
panic(const char *s)
{
    // only block the process who in panic
    kputs("PANIC:");
    kputs(s);
    while(1)
        ;
}
