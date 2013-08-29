#include "include/mmu.h"
#include "include/trap.h"


.globl  alltraps
.globl  restart


alltraps:
    pushal
    pushl   %ds
    pushl   %es
    pushl   %fs
    pushl   %gs

    movw    $(SEG_KDATA << 3), %ax
    movw    %ax, %ds           //cs & ss has been seted by hw
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    //movb    $EOI, %al
    //outb    %al, $INT_M_CTL

    //kstack top is blow the trapframe,
    //does not need to switch        See. allproc

    movl    %esp, %eax
    pushl   %eax        //trapframe as argument
    call    trap
    addl    $4, %esp



restart:
    /*
    movl    current, %eax
    movl    (%eax), %edx
    movl    $tss, %ecx
    movl    %edx, 4(%ecx) //4: offset of sp0 in ts
                            // tss->sp0 <-- current->tfend

    movl    4(%eax), %esp   //%esp <-- current->tf*/
    /*movl    current, %eax
    movl    (%eax), %esp
    */
    cli
    popl    %gs
    popl    %fs
    popl    %es
    popl    %ds
    popal

    addl    $8, %esp   //errcode & trapno

    iret

