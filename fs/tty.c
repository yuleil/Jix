#include "tty.h"
#include "ctype.h"
#include "def.h"
#include "trap.h"
#include "global.h"

struct s_tty tty;
static u8 current_color;

void
set_cursor(u32 pos)
{
    outb(CRTC_ADDR_REG, CURSOR_H);
    outb(CRTC_DATA_REG, (pos >> 8) | 0xff);
    outb(CRTC_ADDR_REG, CURSOR_L);
    outb(CRTC_DATA_REG, pos | 0xff);
}

void
tty_init(void)
{
    tty.count = 0;
    tty.buf_head = 0;
    tty.base = (u16 *)VIDEO_BASE;
    tty.pos = 0;
    tty.mode = TTY_ECHO | TTY_LINE;
    current_color = DEFAULT_COLOR;
    enable_irq(IRQ_KBD);
}



void
tty_putc(int ch)
{
    if (ch == '\n')
        tty.pos += 80 - tty.pos % 80;
    else if (ch == '\b' && tty.pos > 0)
        tty.base[--tty.pos] = ' ' | (current_color << 8);
    else //if (isprint(ch))
        tty.base[tty.pos++] = (ch & 0xff) | (current_color << 8);

    if (tty.pos >= 23 * 80) {  //  keep the bottom
        memcpy(tty.base, tty.base + 80,
               sizeof(tty.base[0]) * 23 * 80);
        tty.pos -= 80;
        memset(tty.base + tty.pos, 0,
               sizeof(tty.base[0]) * (24*80 - tty.pos));
    }
    set_cursor(tty.pos);
}

void
tty_cls()
{
    int i;
    for (i = 0; i < 24 * 80; i++)
        tty.base[i] = ' ' | (DEFAULT_COLOR << 8);
    set_cursor(tty.pos = 0);
}

int
tty_gotoxy(u32 x, u32 y)
{
    if (x > 79 || y > 23)
        return -1;
    set_cursor(tty.pos = y * 80 + x);
    return 0;
}

void
tty_setcolor(back, front)
{
    current_color = MAKE_COLOR((u8)back, (u8)front);
}

void
tty_setmode(u32 mode)
{
    tty.mode = mode & (TTY_LINE | TTY_ECHO);
}

#define C(x)  ((x)-'@')  // Control-x

int
tty_read(char *dst, u32 n)
{
    u32 target;
    int c;

    target = n;

    if (tty.count == 0) {
        fs_sleep = &tty;
        return -2; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }

    disable_irq(IRQ_KBD);
    while (n > 0 && tty.count) {
        c = tty.in_buf[tty.buf_head];
        if (c == C('D')) {
            // if nothing read && eof, eat the EOF
            // else keep eof in input buffer
            if (n == target) {
                tty.buf_head = (tty.buf_head + 1) % BUFSIZE;
                tty.count--;
            }
            break;
        }

        tty.buf_head = (tty.buf_head + 1) % BUFSIZE;
        tty.count--;

        *dst++ = c;
        --n;
    }
    enable_irq(IRQ_KBD);
    return target - n;
}

int
tty_write(char *src, u32 n)
{
    int i;
    for(i = 0; i < n; i++)
        tty_putc(src[i]);

    return n;
}




void
ttyintr(void)
{
    int c;

    while ((c = kbdgetc()) >= 0) {
        switch (c) {
            case C('C'):
                break;
            case '\b':
                if (!(tty.mode & TTY_LINE))
                    goto L1;
                if (tty.count) {
                    if (tty.mode & TTY_ECHO)
                        tty_putc('\b');
                    tty.count--;
                }
                break;
            default:
L1:
                if (c && tty.count < BUFSIZE - 1) {
                    tty.in_buf[(tty.buf_head + tty.count++)
                            % BUFSIZE] = c;
                    if ((tty.mode & TTY_ECHO))
                        tty_putc(c);

                    if (c == '\n' || c == C('D')) {
                        wake_up(&tty);
                    }
                    else if (!(tty.mode & TTY_LINE))
                        wake_up(&tty);

                }
                break;
        }
    }
}
