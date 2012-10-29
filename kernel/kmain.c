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

void hello_world(void)
{
	restore_cr0();
	apic_enable();
	lapic_init();

	cpu_sync(maximum_cpu);

	interrupts_start();

	sched_start_timer();
	sched_idle_loop();
}

void kmain(struct mboot *mboot, unsigned int magic)
{
	char buf[64];
	gdt_init();
	screen_clear();
	mboot_parse(mboot);
	mboot_parse_cmdline(mboot);
	cpu_find();
	interrupts_init();

	syscalls_init();
	drivers_init();
	proc_create_kernel_proc();
	scheduling_init();

	mboot_load_modules(mboot);

	timer_init();
	interrupts_start();

	screen_putstr(kprintf(buf, "MAX_CPU:%x\n", maximum_cpu));

	cpu_wake_all(maximum_cpu);
	cpu_sync(maximum_cpu);

	sched_start_timer();
	sched_idle_loop();
}
