#include <iostream>
#include "m6502.h"

using namespace std;


void print_machine(m6502* m) {}


int main() {
   m6502* m = new m6502;
   m->init(true);

   cout << "hello world";
}
