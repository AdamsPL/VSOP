#include "scheduler.h"

#include "cpu.h"
#include "screen.h"
#include "util.h"
#include "multitasking.h"

struct Scheduler
{
};

struct Scheduler schedulers[MAX_CPU];

static void init(struct Scheduler *this)
{
}

void sched_init_all()
{
	int i;
	for (i = 0; i < MAX_CPU; ++i)
		init(schedulers + i);

	lapic_set(0x320, 0x20080);
	lapic_set(0x3E0, 0xB);
	lapic_set(0x380, 0x43000000);
}

struct Scheduler *sched_current()
{
	return schedulers + cpuid();
}

static void switch_stack(uint32 addr)
{
	asm volatile("movl %0, %%ebp" :: "r"(addr));
	asm ("hlt");
}

void sched_tick(struct Scheduler *this)
{
	char buf[128];
	screen_putstr(kprintf(buf, "%x tick!\n", this));
	switch_stack(threads[1]->kernel_stack);
}

