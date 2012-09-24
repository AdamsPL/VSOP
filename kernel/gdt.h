#ifndef _GDT_H
#define _GDT_H

#include "config.h"

struct gdt
{
	uint16 size;
	uint32 base;
}__attribute__((packed));

void gdt_init();
void tss_set_stack(uint32 cpu, uint32 stack);
void tss_flush(uint32 id);

#endif 


