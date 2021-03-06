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
uint8 maximum_cpu = 8;

#define INIT 		0x00000500
#define STARTUP		0x00000600
#define LEVEL 		0x00008000
#define ASSERT		0x00004000
#define DEASSERT	0x00000000

#define CR0_PAGING 			(1 << 31)
#define CR0_PROTECTED_MODE 	(1 << 0)

uint32 cpu_stack[MAX_CPU];
uint8 cpu_mapping[MAX_CPU];

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
	struct MP_config *mpc = mp_find()->config;
	uint8 *ptr;
	int i;

	num_of_cpu = 0;

	cpu_mapping[0] = 0;

	ptr = (uint8*)mpc + sizeof(*mpc);
	for (i = 0; i < mpc->entry_count; ++i) {
		if (*ptr == 0){
			struct MP_proc_entry *pe_ptr = (struct MP_proc_entry*)ptr;
			cpu_stack[pe_ptr->lapic_id] = (uint32)kmalloc(PAGE_SIZE) + PAGE_SIZE - 0x10;
			cpu_mapping[i] = pe_ptr->lapic_id;
			ptr += 20;
			++num_of_cpu;
		}else
			break;
	}
	if (num_of_cpu < maximum_cpu)
		maximum_cpu = num_of_cpu;
}

void cpu_wake_all(int count)
{
	char buf[256];
	uint16 *warm_reset_vector = (uint16*)0x1000;
	uint32 addr = 0x50000;
	uint32 *trampoline = (uint32*)addr;
	uint32 trampoline_len = (uint32)_cpu_trampoline_end - (uint32)_cpu_trampoline;
	uint32 fake_gdt_len = (uint32)&fake_gdt_end - (uint32)&fake_gdt_ptr;
	int cpu;
	int id;
	struct gdt *virt_gdt_ptr;

	orig_cr0 = CR0_PAGING | CR0_PROTECTED_MODE;
	restore_cr0();

	paging_map((uint32)trampoline, addr, PAGE_WRITABLE | PAGE_PRESENT);

	kmemcpy((uint8*)trampoline, (uint8*)(uint32)(_cpu_trampoline), trampoline_len);
	kmemcpy((uint8*)trampoline + PAGE_SIZE, (uint8*)&fake_gdt_ptr, fake_gdt_len);
	addr = paging_get_phys((uint32)trampoline);

	virt_gdt_ptr = (struct gdt *)((uint8*)trampoline + PAGE_SIZE);
	virt_gdt_ptr->base = addr + PAGE_SIZE + sizeof(struct gdt);
	
	port_write(0x70, 0x0F);
	port_write(0x71, 0x0A);

	paging_map((uint32)warm_reset_vector, (0x467), PAGE_PRESENT | PAGE_WRITABLE);

	warm_reset_vector[0] = (uint16)(addr & 0xf);
	warm_reset_vector[1] = (uint16)(addr >> 4);

	for (cpu = 1; cpu < count; ++cpu)
	{
		id = cpu_mapping[cpu];
		screen_putstr(kprintf(buf, "wake! cpu %i lapic_id:%x\n", cpu, id));
		lapic_set(LAPIC_ICR_HIGH, id << 24);
		lapic_set(LAPIC_ICR_LOW, INIT | LEVEL | ASSERT);
		timer_active_wait(1);
		lapic_set(LAPIC_ICR_HIGH, id << 24);
		lapic_set(LAPIC_ICR_LOW, INIT | LEVEL | DEASSERT);
		timer_active_wait(1);
		lapic_set(LAPIC_ICR_HIGH, id << 24);
		lapic_set(LAPIC_ICR_LOW, STARTUP | (addr >> 12));
		timer_active_wait(1);
	}
	screen_putstr(kprintf(buf, "WAKEUP DONE\n"));
	return;
}

int cpu_count()
{
	return num_of_cpu;
}

void cpu_sync(int count)
{
	section_enter(&lock);
	++awoken_cpu;
	section_leave(&lock);
	while(awoken_cpu != count)
		;
}
