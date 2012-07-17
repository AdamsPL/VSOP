#ifndef _GDT_H
#define _GDT_H

#include "config.h"

void gdt_init();
void tss_set_stack(uint32 cpu, uint32 stack);

#endif 


