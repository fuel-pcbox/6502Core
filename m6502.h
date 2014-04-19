#ifndef M6502_H
#define M6502_H

#include "common.h"

struct m6502
{
  //Registers

  extern u8 a,x,y,s;
  extern u8 flags;
  extern u16 pc;
  
  //Read/Write callbacks
  
  extern function<u8(u16)> rb;
  extern function<void(u16,u8)> wb;
  
  //Pin callbacks
  extern function<bool()> sync_r; //ONLY HERE BECAUSE APPLE 3 WILL USE IT.
  
  extern function<void(bool)> reset_w;
  
  extern function<void(bool)> irq_w;
  extern function<void(bool)> nmi_w;
  
  extern function<void(bool)> rdy_w;

  extern function<void(bool)> so_w;
  
  void init();
  
  void tick();
};

#endif //M6502_H