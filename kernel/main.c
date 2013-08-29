#include "def.h"



int
main()
{
    kpageinit();
    seginit();
    init_8259A();
    idtinit();
    kallocinit();
    taskinit();
    userinit();

    intrinit();

    kputs("scheduling..");
    schedule();

    // never return
    return 0;
}


