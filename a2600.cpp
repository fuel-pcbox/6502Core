#include "m6502.h"
#include "tia.h"

#include <cstdio>

int main(int ac, char** av)
{
    if(ac<2) return 1;
    FILE* fp = fopen(av[1],"rb");

    u8 rom[0x1000];
    u8 ram[0x80];

    fread(rom,1,0x800,fp);
    fseek(fp,0,SEEK_SET);
    fread(rom+0x800,1,0x800,fp);

    fclose(fp);

    m6502 cpu;
    tia gfx;
    gfx.init(&cpu);
    cpu.rb = [&](u16 addr) -> u8
    {
        addr = addr & 0x1FFF;
        if((addr & 0x1280) == 0x0080)
        {
            return ram[addr & 0x7F];
        }
        else if((addr & 0x1080) == 0x0000)
        {
            return 0xFF;
        }
        else if(addr & 0x1000)
        {
            return rom[addr - 0x1000];
        }
        else return 0xFF;
    };

    cpu.wb = [&](u16 addr, u8 value)
    {
        addr = addr & 0x1FFF;
        if((addr & 0x1280) == 0x0080)
        {
            ram[addr & 0x7F] = value;
        }
        else if((addr & 0x1080) == 0x0000)
        {
            switch(addr & 0x3F)
            {
            case 0x02:
            {
                gfx.rdycounter = 160 - (228 - gfx.x);
                if(gfx.rdycounter < 0) gfx.rdycounter += 160;
                cpu.rdy = false;
                break;
            }
            }
        }
    };

    cpu.init();
    for(int i = 0; i<16000; i++)
    {
        cpu.tick();
        gfx.tick();
        printf("op=%02X\n",cpu.op);
        printf("cycle=%d\n",i);
        printf("a=%02X\n",cpu.a);
        printf("x=%02X\n",cpu.x);
        printf("y=%02X\n",cpu.y);
        printf("s=%02X\n",cpu.s);
        printf("p=%02X\n",cpu.flags);
        printf("pc=%04X\n",cpu.pc);
    }
}