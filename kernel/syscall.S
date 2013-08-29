#include "trap.h"

.globl sys_msg

sys_msg:
    pushl   %ebx
    pushl   %ecx
    pushl   %edx

    movl    16(%esp), %ebx  //arg1
    movl    20(%esp), %ecx  //arg2
    movl    24(%esp), %edx  //arg3
    int     $T_SYSCALL

    popl    %edx
    popl    %ecx
    popl    %ebx

    ret
