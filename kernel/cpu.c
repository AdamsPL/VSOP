#include "cpu.h"

#include "gdt.h"
#include "memory.h"
#include "interrupts.h"

static void *mp_slide(uint8 *from, uint8 *to)
{
	while(from < to){
		struct MP_float_ptr *ptr = (struct MP_float_ptr*)from;
		if (ptr->signature[0] == '_'
			&& ptr->signature[1] == 'M'
			&& ptr->signature[2] == 'P'
			&& ptr->signature[3] == '_'
			){
			return from;
		}
		from++;
	}
	return 0;
}

struct MP_float_ptr *mp_find()
{
	/*const uint32 *ebda = (uint32*)0x040E;
	void *base = (*ebda) << 4;*/
	void *mp_tab;

	/*
	if (mp_tab = mp_slide(base, base + 1024))
		return mp_tab;
	*/
	if ((mp_tab = mp_slide((uint8*)(639*1024), (uint8*)(640*1024))))
		return mp_tab;

	if ((mp_tab = mp_slide((uint8*)0xF0000, (uint8*)0xFFFFF)))
		return mp_tab;

	return 0;
}

uint32 cpuid()
{
	return 0;
	/*return lapic_get(LAPIC_ID);*/
}

uint32 esp(void)
{
	asm("movl %esp, %eax");
}
