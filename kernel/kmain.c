#include "screen.h"
#include "memory.h"
#include "util.h"
#include "cpu.h"
#include "interrupts.h"
#include "paging.h"
#include "locks.h"
#include "ipc.h"
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
	lapic_init();
	sched_init();
	interrupts_start();
}

void kmain(struct mboot *mboot, unsigned int magic)
{
	uint32 eflags;

	gdt_init();
	screen_clear();
	mboot_parse(mboot);
	cpu_find();
	interrupts_init();
	syscalls_init();
	drivers_init();
	asm("pushf");
	asm("pop %eax");
	asm("movl %%eax, %0" : "=a"(eflags));
	eflags |= 1 << 14;
	asm("movl %0, %%eax" :: "m"(eflags));
	asm("push %eax");
	asm("popf");

	proc_create_kernel_proc();

	sched_init();
	timer_init();
	interrupts_start();

	cpu_wake_all();
	
	mboot_load_modules(mboot);

	while(1)
		asm("hlt");

}
