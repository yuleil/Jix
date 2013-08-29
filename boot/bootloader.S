#define SEG_DESC(type, base, lim)\
    .word ( ((lim) >> 12) & 0xffff), ((base) & 0xffff);\
    .byte ( ((base) >> 16) & 0xff), (0x90 | (type)),\
    (0xc0 | (((lim) >> 28) & 0xf) ), (((base) >> 24) & 0xff)
//0xc0 because limit is 4096-byte units

#define STA_X   0x8
#define STA_E   0x4
#define STA_C   0x4
#define STA_W   0x2
#define STA_R   0x2
#define STA_A   0x1

#define CR0_PE  0x1

#define SEG_KCODE   1
#define SEG_KDATA   2
#define SEG_VIDEO   3
#define SEG_NULL    0


.code16
.global start

start:
    cli

    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

# open A20 address line

L1:
    inb     $0x64, %al       # waitting...
    testb   $0x2,  %al
    jnz     L1

    movb    $0xd1, %al
    outb    %al, $0x64
L2:
    inb     $0x64, %al
    testb   $0x2,  %al
    jnz     L2

    movb    $0xdf, %al
    outb    %al, $0x60


# switch to PM
    lgdt    gdtdesc
    movl    %cr0, %eax
    orl     $CR0_PE, %eax
    movl    %eax, %cr0

    ljmp    $(SEG_KCODE<<3), $start32

.code32
start32:
    movw    $(SEG_KDATA<<3), %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
#    xorw    %ax, %ax    //NULL SEG
    movw    %ax, %fs
    movw    $(SEG_VIDEO<<3), %ax
    movw    %ax, %gs

    movl    $start, %esp    // use blow 0x7c00 as stack
    call    bootmain

spin:
    jmp     spin

.p2align 2
gdt:
    SEG_DESC(0x0, 0x0, 0x0)                 //null
    SEG_DESC(STA_X|STA_R, 0x0, 0xffffffff)  //code
    SEG_DESC(STA_W, 0x0, 0xffffffff)        //data
    SEG_DESC(STA_W, 0x800b8000, 0xffff)
gdtdesc:
  .word     (gdtdesc - gdt - 1)
  .long     gdt
