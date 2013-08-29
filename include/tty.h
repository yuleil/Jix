#ifndef TTY_H
#define TTY_H

#include "types.h"

#define BUFSIZE 1024

struct s_tty {
    u16 *base;
    u32 pos;
    u8  in_buf[BUFSIZE];
    u32 buf_head;
    u32 count;
    u32 mode;
};

#define TTY_ECHO 0x1
#define TTY_LINE 0x2

#define	CRTC_ADDR_REG	0x3D4
#define	CRTC_DATA_REG	0x3D5
#define	START_ADDR_H	0xC
#define	START_ADDR_L	0xD
#define	CURSOR_H	0xE
#define	CURSOR_L	0xF


#define BLACK   0x0
#define WHITE   0x7
#define RED     0x4
#define GREEN   0x2
#define BLUE    0x1
#define FLASH   0x80
#define BRIGHT  0x08
#define	MAKE_COLOR(x,y)	((x<<4) | y)
// bg, fg

#define VIDEO_BASE      0x800b8000u
#define DEFAULT_COLOR   MAKE_COLOR(BLACK, WHITE)

#endif // TTY_H
