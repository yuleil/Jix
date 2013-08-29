#include "global.h"
#include "msg.h"
#include "def.h"
#include "usr.h"
#include "fs.h"

void
task_sys() // 1
{
    struct msg m;

    kputs("task_sys running...");
    sys_msg(RECV, &m, ANY);
}
