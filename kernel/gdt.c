#include "gdt.h"
#include "util.h"
#include "interrupts.h"

#define GDT_ENTRIES (6 + MAX_CPU)

struct gdt_entry
{
	uint16 size_low;
	uint16 base_low;
	uint8 base_middle;
	uint8 access;
	uint8 granularity;
	uint8 base_high;
} __attribute__((packed));


struct tss_entry
{
	uint32 prev_tss;
	uint32 esp0;
	uint32 ss0;
	uint32 esp1;
	uint32 ss1;
	uint32 esp2;
	uint32 ss2;
	uint32 cr3;
	uint32 eip;
	uint32 eflags;
	uint32 eax;
	uint32 ecx;
	uint32 edx;
	uint32 ebx;
	uint32 esp;
	uint32 ebp;
	uint32 esi;
	uint32 edi;
	uint32 es;
	uint32 cs;
	uint32 ss;
	uint32 ds;
	uint32 fs;
	uint32 gs;
	uint32 ldt;
	uint16 trap;
	uint16 iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[GDT_ENTRIES];
struct gdt gdtr;
static struct tss_entry tss[MAX_CPU];

extern void gdt_flush(uint32 ptr);

static void gdt_set(int32 id, uint32 base, uint32 limit, uint8 access, uint8 gran)
{
	gdt[id].base_low = (base & 0xFFFF);
	gdt[id].base_middle = (base >> 16) & 0xFF;
	gdt[id].base_high = (base >> 24) & 0xFF;

	gdt[id].size_low = (limit & 0xFFFF);
	gdt[id].granularity = (limit >> 16) & 0x0F;

	gdt[id].granularity |= gran & 0xF0;
	gdt[id].access = access;
}

void tss_flush(uint32 id)
{
	id += 5;
	id *= 8;
	id |= 0x03;
	asm("movl %0, %%eax" :: "m"(id) : "%eax");
	asm("ltr %ax");
}

static void tss_init(uint8 id)
{
	gdt_set(5 + id, VIRT_TO_PHYS(&tss[id]), VIRT_TO_PHYS(&tss[id + 1]), 0xE9, 0x00);
	tss[id].ss0 = 0x10;
	tss[id].esp0 = 0xDEADC0DE;
	tss[id].cs = 0x0b;
	tss[id].ss = tss[id].ds = tss[id].es = tss[id].fs = tss[id].gs = 0x13;
}

void gdt_init()
{
	int i = 0;
	gdtr.size = sizeof(gdt) - 1;
	gdtr.base = VIRT_TO_PHYS(&gdt);
	
	kmemset((uint8*)&tss, 0, sizeof(tss));

	gdt_set(0, 0, 0, 0, 0);
	gdt_set(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	
	for (i = 0; i < MAX_CPU; ++i)
		tss_init(i);

	gdt_flush((uint32)&gdtr);
}

void tss_set_stack(uint32 cpu, uint32 stack)
{
	tss[cpu].esp0 = stack;
}
