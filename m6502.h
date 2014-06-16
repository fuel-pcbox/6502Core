#ifndef M6502_H
#define M6502_H

#include "common.h"

enum CycleType
{
    Read,
    Write
};

struct m6502
{
    //Registers

    u8 a,x,y,s;
    u8 flags;
    u16 pc;

    bool sync;
    bool reset;
    bool irq;
    bool nmi;
    bool rdy;
    bool so;

    bool execing;

    bool phi2;

    int cycle;
    CycleType cycletype;

    u8 op;

    u8 tmp1;
    u8 tmp2;
    u16 tmp3;
    u16 tmp4;

    //Read/Write callbacks

    function<u8(u16)> rb;
    function<void(u16,u8)> wb;

    void init();

    void tick();
    void halftick();
};

#endif //M6502_H
