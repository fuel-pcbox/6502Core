#include "stdafx.h"

void vicii::init(m6502* pcpu)
{
    phi2 = false;
    cycle = 0;
    y = 0;
    update = false;
    cpu = pcpu;
}

void vicii::halftick()
{
    tick();
    phi2 = !phi2;
}

void vicii::tick()
{
    update = false;
    cycle++;
    if(cycle >= 127)
    {
        y++;
        cycle = 0;
        if(y >= 262)
        {
            y = 0;
            update = true;
        }
    }
    x = cycle << 2; //* 4
    if((y & 7) == (regs[0x11]&7))
    {
    }
    else
    {
        switch(regs[0x21] & 0xF)
        {
        case 0x6:
        {
            screen[((x+(y*411))<<2)+0] = 0;
            screen[((x+(y*411))<<2)+1] = 0;
            screen[((x+(y*411))<<2)+2] = 128;
            break;
        }
        }
    }
}