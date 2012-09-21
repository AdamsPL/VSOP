#include "cpu.h"

#include "gdt.h"
#include "memory.h"
#include "interrupts.h"
#include "screen.h"
#include "ports.h"

static void *mp_slide(uint8 *from, uint8 *to)
{
	while(from < to)
	{
		struct MP_float_ptr *ptr = (struct MP_float_ptr*)from;
		if (ptr->signature[0] == '_'
			&& ptr->signature[1] == 'M'
			&& ptr->signature[2] == 'P'
			&& ptr->signature[3] == '_')
			return from;
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
	//return lapic_get(LAPIC_ID) >> 24;
}

uint32 esp(void)
{
	asm("movl %esp, %eax");
}

uint32 cpu_count = 1;

void cpu_find()
{
	char buf[256];
	struct MP_config *mpc = mp_find()->config;
	uint8 *ptr;
	int i;

	ptr = (uint8*)mpc + sizeof(*mpc);
	for (i = 0; i < mpc->entry_count; ++i) {
		if (*ptr == 0){
			struct MP_proc_entry *pe_ptr = (struct MP_proc_entry*)ptr;
			screen_putstr(kprintf(buf, "lapic id:%i ", pe_ptr->lapic_id));
			screen_putstr(kprintf(buf, "cpu_flags:%x\n", pe_ptr->cpu_flags));
			ptr += 20;
			++cpu_count;
		}else
			break;
	}
}

void _cpu_trampoline(void);
void _cpu_trampoline_end(void);
extern uint32 fake_gdt_ptr;
extern uint32 fake_gdt_end;

#define INIT 	0x00000500
#define STARTUP	0x00000600
#define LEVEL 	0x00008000
#define ASSERT	0x00004000

uint32 cpu_stack;

void cpu_wake_all()
{
	char buf[256];
	uint32 *warm_reset_vector = (uint32*)(0x40 << 4 | 0x67);
	uint32 *trampoline = kmalloc(PAGE_SIZE * 2);
	uint32 trampoline_len = _cpu_trampoline_end - _cpu_trampoline;
	uint32 addr;
	uint32 fake_gdt_len = (uint32)&fake_gdt_end - (uint32)&fake_gdt_ptr;
	int cpu;
	struct gdt *virt_gdt_ptr;

	kmemcpy((uint8*)trampoline, (uint8*)_cpu_trampoline, trampoline_len);
	kmemcpy((uint8*)trampoline + PAGE_SIZE, (uint8*)&fake_gdt_ptr, fake_gdt_len);
	addr = paging_get_phys((uint32)trampoline);

	virt_gdt_ptr = (struct gdt *)((uint8*)trampoline + PAGE_SIZE);
	virt_gdt_ptr->base = addr + PAGE_SIZE + sizeof(struct gdt);
	
	port_write(0x70, 0x0F);
	port_write(0x71, 0x0A);
	*warm_reset_vector = addr;

	screen_putstr(kprintf(buf, "trampoline @ %x\n", addr));
	screen_putstr(kprintf(buf, "trampoline len: %x\n", trampoline_len));
	screen_putstr(kprintf(buf, "fake_gdt_len @ %x\n", fake_gdt_len));

	cpu_stack = 0xB00B1E50;
	for (cpu = 1; cpu < cpu_count; ++cpu)
	{
		cpu_stack = (uint32)kmalloc(PAGE_SIZE);
		screen_putstr(kprintf(buf, "cpu %i stack: %x\n", cpu, cpu_stack));
		lapic_set(LAPIC_ICR_HIGH, cpu << 24);
		lapic_set(LAPIC_ICR_LOW, INIT | LEVEL | ASSERT);
		timer_active_wait(200);

		lapic_set(LAPIC_ICR_HIGH, cpu << 24);
		lapic_set(LAPIC_ICR_LOW, STARTUP | (addr >> 12));
	}
	return;
}
