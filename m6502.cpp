#include "m6502.h"

void m6502::init()
{
    reset = true;
    rdy = true;
    cycle = 0;
    execing = false;
    cycletype = CycleType::Read;

    //Following are from the Visual 6502 project.
    a = 0xAA;
    x = 0;
    y = 0;
    s = 0;
    flags = 0x02;
    op = 0;
    pc=0;
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
        op = rb(pc);
        pc++;
        sync = true;
        execing = true;
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
            cycle++;
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
            cycle++;
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
            cycle++;
            execing = false;
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
            cycle++;
            execing = false;
            cycletype = CycleType::Read;
            break;
        }
        }
        break;
    }
    }
}
