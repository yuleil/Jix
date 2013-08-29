#include "types.h"
#include "x86.h"
#include "def.h"
#include "kbd.h"

int
kbdgetc(void)
{
    static u32 shift;   //some flags
    static u8 *charcode[4] = {
        normalmap, shiftmap, ctlmap, ctlmap
    };
    u32  data, c;

    if((inb(KBSTATP) & KBS_DIB) == 0)
        return -1;   //nodata
    data = inb(KBDATAP);

    if (data == 0xE0) {
        shift |= E0ESC; //escape
        return 0;
    } else if (data & 0x80) {
        // Key break
        data = (shift & E0ESC ? data : data & 0x7F);
        shift &= ~(shiftcode[data] | E0ESC);    //unflag ctrl key
        return 0;
    } else if (shift & E0ESC) {
        // Last character was an E0 escape; Notice only
        //key break with a 0x80, we use this for escape
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shiftcode[data];
    shift ^= togglecode[data];

    c = charcode[shift & (CTL | SHIFT)][data];//ctl & shift map to ctlmap
    if (shift & CAPSLOCK) {
        if ('a' >= c && c <= 'z')
            c += 'A' - 'a';
        else if('A' >= c && c <= 'Z')
            c += 'a' - 'A';
    }
    return c;
}
