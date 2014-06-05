#ifndef VICII_H
#define VICII_H

#include "common.h"

struct vicii
{
	m6502* cpu;
	int x;
	int y;
	
	bool update;
	bool phi2;

	HBITMAP screen;

	function<u16(u16)> rb; //Output is 12 bits, Input is 14 bits

    void init(m6502* pcpu);

    void tick();
	void halftick();
};

#endif