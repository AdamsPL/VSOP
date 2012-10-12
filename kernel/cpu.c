#include "cpu.h"

#include "gdt.h"
#include "memory.h"
#include "interrupts.h"
#include "screen.h"
#include "ports.h"
#include "paging.h"
#include "timer.h"

static volatile int num_of_cpu = 0;
void _cpu_trampoline(void);
void _cpu_trampoline_end(void);
extern uint32 fake_gdt_ptr;
extern uint32 fake_gdt_end;

#define INIT 	0x00000500
#define STARTUP	0x00000600
#define LEVEL 	0x00008000
#define ASSERT	0x00004000

#define CR0_PAGING 			(1 << 31)
#define CR0_PROTECTED_MODE 	(1 << 0)

uint32 cpu_stack[MAX_CPU];

static lock_t lock = 0;
static volatile int awoken_cpu = 0;
static int orig_cr0;


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
	return lapic_get(LAPIC_ID) >> 24;
}

uint32 cr0(void)
{
	uint32 result;
	asm("movl %%cr0, %0" : "=r"(result));
	return result;
}

void restore_cr0()
{
	asm("movl %0, %%cr0" :: "r"(orig_cr0));
}

uint32 esp(void)
{
	int result;
	asm("movl %%esp, %0" : "=r"(result));
	return result;
}

void cpu_find()
{
	char buf[256];
	struct MP_config *mpc = mp_find()->config;
	uint8 *ptr;
	int i;

	num_of_cpu = 0;

	ptr = (uint8*)mpc + sizeof(*mpc);
	for (i = 0; i < mpc->entry_count; ++i) {
		if (*ptr == 0){
			struct MP_proc_entry *pe_ptr = (struct MP_proc_entry*)ptr;
			screen_putstr(kprintf(buf, "lapic id:%i ", pe_ptr->lapic_id));
			screen_putstr(kprintf(buf, "cpu_flags:%x\n", pe_ptr->cpu_flags));
			ptr += 20;
			++num_of_cpu;
		}else
			break;
	}
}

void cpu_wake_all()
{
	char buf[256];
	uint32 *warm_reset_vector = (uint32*)(0x40 << 4 | 0x67);
	uint32 *trampoline = kmalloc(PAGE_SIZE * 2);
	uint32 trampoline_len = (uint32)_cpu_trampoline_end - (uint32)_cpu_trampoline;
	uint32 addr;
	uint32 fake_gdt_len = (uint32)&fake_gdt_end - (uint32)&fake_gdt_ptr;
	int cpu;
	struct gdt *virt_gdt_ptr;

	orig_cr0 = CR0_PAGING | CR0_PROTECTED_MODE;
	restore_cr0();

	kmemcpy((uint8*)trampoline, (uint8*)(uint32)(_cpu_trampoline), trampoline_len);
	kmemcpy((uint8*)trampoline + PAGE_SIZE, (uint8*)&fake_gdt_ptr, fake_gdt_len);
	addr = paging_get_phys((uint32)trampoline);

	virt_gdt_ptr = (struct gdt *)((uint8*)trampoline + PAGE_SIZE);
	virt_gdt_ptr->base = addr + PAGE_SIZE + sizeof(struct gdt);
	
	port_write(0x70, 0x0F);
	port_write(0x71, 0x0A);
	*warm_reset_vector = addr;

	for (cpu = 1; cpu < cpu_count(); ++cpu)
	{
		cpu_stack[cpu] = (uint32)kmalloc(PAGE_SIZE) + PAGE_SIZE;
		screen_putstr(kprintf(buf, "wake! cpu %i stack: %x\n", cpu, cpu_stack[cpu]));
		lapic_set(LAPIC_ICR_HIGH, cpu << 24);
		lapic_set(LAPIC_ICR_LOW, INIT | LEVEL | ASSERT);
		timer_active_wait(100);

		lapic_set(LAPIC_ICR_HIGH, cpu << 24);
		lapic_set(LAPIC_ICR_LOW, STARTUP | (addr >> 12));
	}
	return;
}

int cpu_count()
{
	return num_of_cpu;
}

void cpu_sync(void)
{
	char buf[128];
	screen_putstr(kprintf(buf, "cr0:%x\n", cr0()));
	section_enter(&lock);
	++awoken_cpu;
	section_leave(&lock);
	while(awoken_cpu != num_of_cpu)
		;
}
