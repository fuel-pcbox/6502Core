#include "stdafx.h"

void vicii::init(m6502* pcpu)
{
	screen = CreateBitmap(768,544,1,32,NULL);
	phi2 = false;
	cpu = pcpu;
}

void vicii::halftick()
{
	if(phi2) tick();
	if(!cpu->rdy && !phi2) tick();
	phi2 = !phi2;
}

void vicii::tick()
{
	update = false;
	x++;
	if(x >= 63)
	{
		y++;
		x = 0;
		if(y >= 262)
		{
			y = 0;
			update = true;
		}
	}
}