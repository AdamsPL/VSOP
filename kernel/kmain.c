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

void alice(void)
{
	struct process *bob;
	char buf[128];

	screen_putstr(kprintf(buf, "Alice: %x\n", sched_cur_proc()));
	proc_register(proc_get_by_pid(sched_cur_proc()), "ALICE");

	while(!(bob = proc_get_by_name("BOB")))
	{
		screen_putstr(kprintf(buf, "Alice: waiting...!\n"));
		sched_thread_sleep(1000);
	}

	screen_putstr(kprintf(buf, "Alice: Found BOB: %x!\n", bob->pid));
	ipc_connect(sched_cur_proc(), bob->pid);

	while(1)
		asm("hlt");
}

void bob(void)
{
	struct process *alice;
	char buf[128];

	screen_putstr(kprintf(buf, "Bob: %x\n", sched_cur_proc()));
	proc_register(proc_get_by_pid(sched_cur_proc()), "BOB");

	while(!(alice = proc_get_by_name("ALICE")))
	{
		screen_putstr(kprintf(buf, "Bob: waiting...!\n"));
		sched_thread_sleep(1000);
	}

	screen_putstr(kprintf(buf, "Bob: Found ALICE! %x\n", alice->pid));
	ipc_connect(sched_cur_proc(), alice->pid);

	while(1)
		asm("hlt");
}

void hello_world(void)
{
	lapic_init();
	sched_init();
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
	cpu_find();
	interrupts_init();
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
	
	thread = thread_create(proc_create_kernel_proc(), (uint32)alice, THREAD_KERNEL);
	sched_thread_ready(thread);
	
	thread = thread_create(proc_create_kernel_proc(), (uint32)bob, THREAD_KERNEL);
	sched_thread_ready(thread);


	/*
	mboot_load_modules(mboot);
	*/
	while(1)
		asm("hlt");

}
