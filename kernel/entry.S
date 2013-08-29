# That's the Elf entry point
# Note! We dont enable vm yet
#include "include/memlayout.h"
#include "include/mmu.h"
#include "include/const.h"

.p2align 2
.text
.global _start
_start = entry - KBASE  #V2P


.global entry
entry:

    movl    %cr4, %eax
    orl     $(CR4_PSE), %eax    # enable superpage
    movl    %eax, %cr4

    movl    $(entrypgdir-KBASE), %eax
    movl    %eax, %cr3

    # Turn on paging
    movl    %cr0, %eax
    orl     $(CR0_PG|CR0_WP), %eax
    movl    %eax, %cr0

    movl    $(stack + KSTACKSZ), %esp

    movl    $main, %eax
    jmp     *%eax


.comm	stack , KSTACKSZ
