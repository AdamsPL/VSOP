#include "screen.h"
#include "memory.h"
#include "util.h"
#include "cpu.h"
#include "interrupts.h"
#include "paging.h"
#include "locks.h"
#include "ipc.h"
#include "ports.h"
#include "multitasking.h"
#include "gdt.h"
#include "elf.h"
#include "drivers.h"
#include "screen.h"
#include "process.h"
#include "thread.h"


void dumper()
{
	char buf[128];
	uint32 esp;
	while(1)
	{
		asm("movl %%esp, %0" : "=a"(esp));
		screen_putstr(kprintf(buf, "dump! %x\n", esp));
	}
}

void hello_world(void)
{
	char buf[128];
	screen_putstr(kprintf(buf, "Hello world(%x)! esp:%x\n", lapic_get(LAPIC_ID) >> 24, esp()));
	screen_putstr(kprintf(buf, "Hello world(%x)! esp:%x\n", lapic_get(LAPIC_ID) >> 24, esp()));
	lapic_init();
	interrupts_start();
}

void kmain(struct mboot *mboot, unsigned int magic)
{
	struct thread *thread;
	uint32 esp;
	uint32 eflags;
	char buf[128];

	gdt_init();
	screen_clear();
	mboot_parse(mboot);
	interrupts_init();
	drivers_init();
	sched_init_all();
	asm("pushf");
	asm("pop %eax");
	asm("movl %%eax, %0" : "=a"(eflags));
	eflags |= 1 << 14;
	asm("movl %0, %%eax" :: "m"(eflags));
	asm("push %eax");
	asm("popf");
	timer_init();
	interrupts_start();

	cpu_find();
	cpu_wake_all();

	while(1)
		asm("hlt");

	mboot_load_modules(mboot);

	thread = thread_create(proc_create_kernel_proc(), (uint32)dumper, THREAD_KERNEL);
	sched_thread_ready(thread);
	thread = thread_create(proc_create_kernel_proc(), (uint32)dumper, THREAD_KERNEL);
	sched_thread_ready(thread);
	thread = thread_create(proc_create_kernel_proc(), (uint32)dumper, THREAD_KERNEL);
	sched_thread_ready(thread);

	while(1)
		asm("hlt");
}
