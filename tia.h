#ifndef TIA_H
#define TIA_H

#include "common.h"
#include "m6502.h"

struct tia
{
    m6502* cpu;
    int x;
    int y;
    int rdycounter;

    u8 outregs[0x3F];
    u8 inregs[0x0F];

    bool update;

    u8 screen[160*262*4];

    void init(m6502* pcpu);

    void tick();
};

#endif