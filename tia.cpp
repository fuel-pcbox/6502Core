#include "tia.h"

void tia::init(m6502* pcpu)
{
    x = 0;
    y = 0;
    update = false;
    cpu = pcpu;
    rdycounter = 0;
}

void tia::tick()
{
    update = false;
    x++;
    if(rdycounter)
    {
        rdycounter--;
        if(!rdycounter) cpu->rdy = true;
    }
    if(x >= 227)
    {
        y++;
        x = 0;
        if(y >= 261)
        {
            y = 0;
            update = true;
        }
    }
}