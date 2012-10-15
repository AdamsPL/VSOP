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
	restore_cr0();
	apic_enable();
	ioapic_init();
	lapic_init();
	sched_init();
	cpu_sync();

	interrupts_start();

	sched_start_timer();
}

void dupa1(void)
{
	/*
	char buf[128];
	*/
	while(1)
	{
		/*screen_putstr(kprintf(buf, "%x\n", 1));*/
	}
}
void dupa2(void)
{
	/*
	char buf[128];
	*/
	while(1)
	{
		/*screen_putstr(kprintf(buf, "%x\n", 2));*/
	}
}
void dupa3(void)
{
	/*
	char buf[128];
	*/
	while(1)
	{
		/*screen_putstr(kprintf(buf, "%x\n", 3));*/
	}
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
	sched_init();
	/*
	mboot_load_modules(mboot);
	*/
	timer_init();

	interrupts_start();

	cpu_wake_all();

	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa1, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa1, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa1, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa1, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa2, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa2, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa2, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa2, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa3, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa3, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa3, THREAD_KERNEL));
	sched_thread_ready(thread_create(proc_get_by_pid(0), (uint32)dupa3, THREAD_KERNEL));

	cpu_sync();

	sched_start_timer();

	while(1)
		asm("hlt");

}
