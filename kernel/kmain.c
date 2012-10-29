#include "screen.h"
#include "memory.h"
#include "util.h"
#include "cpu.h"
#include "interrupts.h"
#include "paging.h"
#include "locks.h"
#include "ports.h"
#include "gdt.h"
#include "elf.h"
#include "drivers.h"
#include "screen.h"
#include "process.h"
#include "thread.h"
#include "process.h"
#include "scheduler.h"
#include "syscall.h"
#include "timer.h"

/*#define CPU_COUNT (cpu_count())*/
#define CPU_COUNT 2

void hello_world(void)
{
	restore_cr0();
	apic_enable();
	lapic_init();

	cpu_sync(CPU_COUNT);

	interrupts_start();

	sched_start_timer();
	sched_idle_loop();
}

void kmain(struct mboot *mboot, unsigned int magic)
{
	gdt_init();
	screen_clear();
	mboot_parse(mboot);
	cpu_find();
	interrupts_init();

	syscalls_init();
	drivers_init();
	proc_create_kernel_proc();
	scheduling_init();

	mboot_load_modules(mboot);

	timer_init();
	interrupts_start();

	cpu_wake_all(CPU_COUNT);
	cpu_sync(CPU_COUNT);

	sched_start_timer();
	sched_idle_loop();
}
