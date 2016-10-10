#ifdef WIN32
#include "stdafx.h"
#else
#include "m6502.h"
#endif

void m6502::init(bool _bcd)
{
    reset = true;
    irq = true;
    nmi = true;
    rdy = true;
    cycle = 0;
    execing = false;
    phi2 = false;
    cycletype = CycleType::Read;

    bcd = _bcd;

    a = 0;
    x = 0;
    y = 0;
    s = 0;
    flags = 0x24;
    op = 0;
    pc=0;
}

void m6502::halftick()
{
    if(phi2) tick();
    phi2 = !phi2;
}

void m6502::tick()
{
    if(so) flags |= 0x40;
    if(cycletype == CycleType::Read && !rdy)
        return;
    if(reset)
    {
        switch(cycle)
        {
        case 0:
        case 1:
        case 2:
        {
            rb(pc);
            cycle++;
            cycletype = CycleType::Read;
            break;
        }
        case 3:
        case 4:
        case 5:
        {
            rb(s+0x100);
            s--;
            cycle++;
            cycletype = CycleType::Read;
            break;
        }
        case 6:
        {
            pc = (pc & 0xFF00) | rb(0xFFFC);
            cycle++;
            cycletype = CycleType::Read;
            break;
        }
        case 7:
        {
            pc = (rb(0xFFFD) << 8) | (pc & 0xFF);
            reset = false;
            cycle=0;
            cycletype = CycleType::Read;
            break;
        }
        }
    }
    else if(!execing)
    {
        if(!irq) op = 0x100;
        if(!nmi) op = 0x101;
        else
        {
            op = rb(pc);
            pc++;
            sync = true;
            execing = true;
        }
    }
    else if(execing)
    {
        sync = false;
    }
    if(!reset) switch(op)
        {
        case 0x00:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 2:
            {
                wb(s+0x100,pc>>8);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(s+0x100,pc&0xFF);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(s+0x100,flags|0x30);
                s--;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                pc = (pc & 0xFF00) | rb(0xFFFE);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 6:
            {
                pc = (rb(0xFFFF) << 8) | (pc & 0xFF);
                execing=false;
                cycle=0;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x01:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp1 = rb(tmp1) + x;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp3 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 4:
            {
                tmp3 |= rb(tmp1+1)<<8;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                a |= rb(tmp3);
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x02:
        case 0x12:
        case 0x22:
        case 0x32:
        case 0x42:
        case 0x52:
        case 0x62:
        case 0x72:
        case 0x92:
        case 0xB2:
        case 0xD2:
        case 0xF2:
        {
            cycle = 8;
            break;
        }
        case 0x03:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp1 = rb(tmp1) + x;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp3 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 4:
            {
                tmp3 |= rb(tmp1+1)<<8;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                tmp1 = rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 6:
            {
                wb(tmp3,tmp1);
                if(tmp1 &= 0x80) flags |= 0x01;
                else flags &= 0xFE;
                tmp1 <<= 1;
                a |= tmp1;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 7:
            {
                wb(tmp3,tmp1);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x04:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(tmp1);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x05:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                a |= rb(tmp1);
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x06:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp2 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp1,tmp2);
                if(tmp2&0x80) flags |= 0x01;
                else flags &= 0xFE;
                if(tmp2&0x40) flags |= 0x80;
                else flags &= 0x7F;
                tmp2 <<= 1;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp1,tmp2);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x07:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp2 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp1,tmp2);
                if(tmp2&0x80) flags |= 0x01;
                else flags &= 0xFE;
                tmp2 <<= 1;
                tmp2 |= a;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp1,tmp2);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x08:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                wb(s+0x100,flags);
                s--;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x09:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a |= rb(pc);
                pc++;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x0A:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                if(a&0x80) flags |= 0x01;
                else flags &= 0xFE;
                if(a&0x40) flags |= 0x80;
                else flags &= 0x7F;
                a <<= 1;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x0B:
        case 0x2B:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a &= rb(pc);
                pc++;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x0C:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                rb(tmp3);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x0D:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                a |= rb(tmp3);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x0E:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp1 = rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp3,tmp1);
                if(tmp1&0x80) flags |= 0x01;
                else flags &= 0xFE;
                if(tmp1&0x40) flags |= 0x80;
                else flags &= 0x7F;
                tmp1 <<= 1;
                if(!tmp1) flags |= 0x02;
                else flags &= 0xFD;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 5:
            {
                wb(tmp3,tmp1);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x0F:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp1 = rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp3,tmp1);
                if(tmp1&0x80) flags |= 0x01;
                else flags &= 0xFE;
                tmp1 <<= 1;
                a |= tmp1;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 5:
            {
                wb(tmp3,tmp1);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x10:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(pc);
                if(!(flags & 0x80))
                {
                    tmp3 = pc + (s8)tmp1;
                    pc = (pc & 0xFF00) | ((pc + (s8)tmp1) & 0xFF);
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 3:
            {
                rb(pc);
                if(pc!=tmp3)
                {
                    pc = tmp3;
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                rb(pc);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x11:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp3 |= (rb(tmp1) << 8);
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | (tmp4 & 0xFF);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 4:
            {
                tmp1 = rb(tmp3);
                if(tmp3 == tmp4)
                {
                    a |= tmp1;
                    if(!a) flags |= 0x02;
                    else flags &= 0xFD;
                    if(a & 0x80) flags |= 0x80;
                    else flags &= 0x7F;
                    cycle=0;
                    execing = false;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 5:
            {
                a |= rb(tmp4);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x18:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                flags &= 0xFE;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x19:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= (rb(pc)<<8);
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp1 = rb(tmp3);
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    a |= tmp1;
                    if(!a) flags |= 0x02;
                    else flags &= 0xFD;
                    if(a & 0x80) flags |= 0x80;
                    else flags &= 0x7F;
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                a |= rb(tmp4);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x20:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(s + 0x100);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(s + 0x100,pc>>8);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(s + 0x100,pc&0xFF);
                s--;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                pc = tmp1 | (rb(pc)<<8);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x24:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp1 = rb(tmp1);
                u8 tmp = a & tmp1;
                if(!tmp) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp1&0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp1&0x40) flags |= 0x40;
                else flags &= 0xbf;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0x25:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp1 = rb(tmp1);
                a &= tmp1;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0x29:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a &= rb(pc);
                pc++;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x2A:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                u8 tmp = a;
                a = (a << 1) | (flags & 1);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp & 0x80) flags |= 0x01;
                else flags &= 0xFE;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x30:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(pc);
                if(flags & 0x80)
                {
                    tmp3 = pc + (s8)tmp1;
                    pc = (pc & 0xFF00) | ((pc + (s8)tmp1) & 0xFF);
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 3:
            {
                rb(pc);
                if(pc!=tmp3)
                {
                    pc = tmp3;
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                rb(pc);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x38:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                flags |= 0x01;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x45:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp1 = rb(tmp1);
                a ^= tmp1;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0x49:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a ^= rb(pc);
                pc++;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x4A:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                if(a&0x1) flags |= 0x01;
                else flags &= 0xFE;
                flags &= 0x7F;
                a >>= 1;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x4C:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                pc = (rb(pc)<<8) | tmp1;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x58:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                flags &= 0xFB;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x60:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(s + 0x100);
                s++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                pc = (pc & 0xFF00) | rb(s + 0x100);
                s++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                pc = (pc & 0xFF) | (rb(s + 0x100) << 8);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                rb(pc);
                pc++;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x69:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                u8 res1 = (a + tmp1 + (flags & 1));
                u16 res2 = (a + tmp1 + (flags & 1));
                s16 res3 = res2;
                a = res1;
                if(flags & 0x08)
                {
                    flags &= 0xfe;
                    if((a & 0x0f) > 0x09) a += 6;
                    if((a & 0xf0) > 0x90)
                    {
                        flags |= 0x01;
                        a += 0x60;
                    }
                }

                if(res1 & 0x80) flags |= 0x80;
                else flags &= 0x7f;
                if(res1 == 0) flags |= 0x02;
                else flags &= 0xfd;
                if(res2 >= 0x100) flags |= 0x01;
                else flags &= 0xfe;
                if(res3 < -128 || res3 > 127) flags |= 0x40;
                else flags &= 0xbf;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x6A:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                u8 tmp = a;
                a = (a >> 1) | (flags << 7);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp & 0x01) flags |= 0x01;
                else flags &= 0xFE;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x78:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                flags |= 0x04;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x84:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 2:
            {
                wb(tmp1,y);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x85:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 2:
            {
                wb(tmp1,a);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x86:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 2:
            {
                wb(tmp1,x);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x88:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                y--;
                if(!y) flags |= 0x02;
                else flags &= 0xFD;
                if(y & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x8A:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a = x;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x8C:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp3,y);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x8D:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp3,a);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x8E:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp3,x);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x90:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(pc);
                if(flags & 0x01)
                {
                    tmp3 = pc + (s8)tmp1;
                    pc = (pc & 0xFF00) | ((pc + (s8)tmp1) & 0xFF);
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 3:
            {
                rb(pc);
                if(pc!=tmp3)
                {
                    pc = tmp3;
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                rb(pc);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x91:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp3 |= rb(tmp1+1)<<8;
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 4:
            {
                rb(tmp3);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                wb(tmp4,a);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x94:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(tmp1);
                tmp1 += x;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp1,y);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0x95:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(tmp1);
                tmp1 += x;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp1,a);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0x98:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a = y;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x99:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp4,a);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x9A:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                s = x;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x9D:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                tmp4 = tmp3 + x;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + x) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp4,a);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xA0:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                y = rb(pc);
                pc++;
                if(!y) flags |= 0x02;
                else flags &= 0xFD;
                if(y & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xA2:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                x = rb(pc);
                pc++;
                if(!x) flags |= 0x02;
                else flags &= 0xFD;
                if(x & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xA4:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                y = rb(tmp1);
                if(!y) flags |= 0x02;
                else flags &= 0xFD;
                if(y&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0xA5:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                a = rb(tmp1);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0xA6:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                x = rb(tmp1);
                if(!x) flags |= 0x02;
                else flags &= 0xFD;
                if(x&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0xA7:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                x = rb(tmp1);
                a = x;
                if(!x) flags |= 0x02;
                else flags &= 0xFD;
                if(x&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0xA8:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                y = a;
                if(!y) flags |= 0x02;
                else flags &= 0xFD;
                if(y & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xA9:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                a = rb(pc);
                pc++;
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xAA:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                x = a;
                if(!x) flags |= 0x02;
                else flags &= 0xFD;
                if(x & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xAC:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                y = rb(tmp3);
                if(!y) flags |= 0x02;
                else flags &= 0xFD;
                if(y & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xAD:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                a = rb(tmp3);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xAE:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                x = rb(tmp3);
                if(!x) flags |= 0x02;
                else flags &= 0xFD;
                if(x & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xB0:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(pc);
                if(flags & 0x01)
                {
                    tmp3 = pc + (s8)tmp1;
                    pc = (pc & 0xFF00) | ((pc + (s8)tmp1) & 0xFF);
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 3:
            {
                rb(pc);
                if(pc!=tmp3)
                {
                    pc = tmp3;
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                rb(pc);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xB1:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp3 |= rb(tmp1+1)<<8;
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 4:
            {
                a = rb(tmp3);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 5:
            {
                a = rb(tmp4);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xB5:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(tmp1);
                tmp1 += x;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                a = rb(tmp1);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a&0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
            }
            }
            break;
        }
        case 0xB9:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= (rb(pc)<<8);
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                a = rb(tmp3);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                a = rb(tmp4);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xBD:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= (rb(pc)<<8);
                tmp4 = tmp3 + x;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + x) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                a = rb(tmp3);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                a = rb(tmp4);
                if(!a) flags |= 0x02;
                else flags &= 0xFD;
                if(a & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xC0:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                tmp2 = y - tmp1;
                pc++;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(y >= tmp1) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xC6:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp2 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp1,tmp2);
                tmp2--;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp1,tmp2);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xC8:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                y++;
                if(!y) flags |= 0x03;
                else flags &= 0xFC;
                if(y & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xC9:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                tmp2 = a - tmp1;
                pc++;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(a >= tmp1) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xCA:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                x--;
                if(!x) flags |= 0x02;
                else flags &= 0xFD;
                if(x & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xCE:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp1 = rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp3,tmp1);
                tmp1--;
                if(tmp1 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(!tmp1) flags |= 0x02;
                else flags &= 0xFD;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 5:
            {
                wb(tmp3,tmp1);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xD0:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(pc);
                if(!(flags & 0x02))
                {
                    tmp3 = pc + (s8)tmp1;
                    pc = (pc & 0xFF00) | ((pc + (s8)tmp1) & 0xFF);
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 3:
            {
                rb(pc);
                if(pc!=tmp3)
                {
                    pc = tmp3;
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                rb(pc);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xD1:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp3 |= rb(tmp1+1)<<8;
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 4:
            {
                tmp2 = a - rb(tmp3);
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp2>a) flags |= 0x01;
                else flags &= 0xFE;
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 5:
            {
                tmp2 = a - rb(tmp4);
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp2>a) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xD8:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                flags |= 0x08;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xD9:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= (rb(pc)<<8);
                tmp4 = tmp3 + y;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + y) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                u8 tmp = a - rb(tmp3);
                if(!tmp) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp>a) flags |= 0x01;
                else flags &= 0xFE;
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                u8 tmp = a - rb(tmp4);
                if(!tmp) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp>a) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xDD:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= (rb(pc)<<8);
                tmp4 = tmp3 + x;
                tmp3 = (tmp3 & 0xFF00) | ((tmp3 + x) & 0xFF);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                u8 tmp = a - rb(tmp3);
                if(!tmp) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp>a) flags |= 0x01;
                else flags &= 0xFE;
                if(tmp3 != tmp4)
                {
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                u8 tmp = a - rb(tmp4);
                if(!tmp) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(tmp>a) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xE0:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                tmp2 = x - tmp1;
                pc++;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(x >= tmp1) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xE4:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
            }
            case 2:
            {
                tmp1 = rb(tmp1);
                tmp2 = x - tmp1;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(x >= tmp1) flags |= 0x01;
                else flags &= 0xFE;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xE5:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp2 = rb(tmp1);
                tmp3 = a - tmp2 - ((flags & 1) ? 0 : 1);
                if(!(flags & 0x08))
                {
                    if(!(u8)tmp3) flags |= 0x02;
                    else flags &= 0xFD;
                    if(tmp3 & 0x80) flags |= 0x80;
                    else flags &= 0x7F;
                    if((a ^ tmp2) & (a ^ tmp3) & 0x80) flags |= 0x40;
                    else flags &= 0xBF;
                    if(!(tmp3>>8)) flags |= 0x01;
                    else flags &= 0xFE;
                }
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xE6:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp2 = rb(tmp1);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(tmp1,tmp2);
                tmp2++;
                if(!tmp2) flags |= 0x02;
                else flags &= 0xFD;
                if(tmp2 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp1,tmp2);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xE8:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                x++;
                if(!x) flags |= 0x03;
                else flags &= 0xFC;
                if(x & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xEA:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xEE:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp3 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                tmp3 |= rb(pc) << 8;
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 3:
            {
                tmp1 = rb(tmp3);
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(tmp3,tmp1);
                tmp1++;
                if(tmp1 & 0x80) flags |= 0x80;
                else flags &= 0x7F;
                if(!tmp1) flags |= 0x02;
                else flags &= 0xFD;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 5:
            {
                wb(tmp3,tmp1);
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xF0:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                tmp1 = rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 2:
            {
                rb(pc);
                if(flags & 0x02)
                {
                    tmp3 = pc + (s8)tmp1;
                    pc = (pc & 0xFF00) | ((pc + (s8)tmp1) & 0xFF);
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 3:
            {
                rb(pc);
                if(pc!=tmp3)
                {
                    pc = tmp3;
                    cycle++;
                    cycletype = CycleType::Read;
                }
                else
                {
                    cycle=0;
                    execing=false;
                    cycletype = CycleType::Read;
                }
                break;
            }
            case 4:
            {
                rb(pc);
                cycle=0;
                execing=false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0xF8:
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                flags &= 0xF7;
                cycle=0;
                execing = false;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x100: //IRQ
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 2:
            {
                wb(s+0x100,pc>>8);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(s+0x100,pc&0xFF);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(s+0x100,flags|0x30);
                s--;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                pc = (pc & 0xFF00) | rb(0xFFFE);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 6:
            {
                pc = (rb(0xFFFF) << 8) | (pc & 0xFF);
                execing=false;
                cycle=0;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        case 0x101: //NMI
        {
            switch(cycle)
            {
            case 0:
            {
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 1:
            {
                rb(pc);
                pc++;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 2:
            {
                wb(s+0x100,pc>>8);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 3:
            {
                wb(s+0x100,pc&0xFF);
                s--;
                cycle++;
                cycletype = CycleType::Write;
                break;
            }
            case 4:
            {
                wb(s+0x100,flags|0x30);
                s--;
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 5:
            {
                pc = (pc & 0xFF00) | rb(0xFFFA);
                cycle++;
                cycletype = CycleType::Read;
                break;
            }
            case 6:
            {
                pc = (rb(0xFFFB) << 8) | (pc & 0xFF);
                execing=false;
                cycle=0;
                cycletype = CycleType::Read;
                break;
            }
            }
            break;
        }
        default:
        {
            printf("Unimplemented opcode %02x!\n",op);
            break;
        }
        }
}
