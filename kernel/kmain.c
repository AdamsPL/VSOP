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

char buf[128];

void dumper()
{
	uint32 esp;
	while(1)
	{
		asm("movl %%esp, %0" : "=a"(esp));
		screen_putstr(kprintf(buf, "dump! %x | ", esp));
		asm("hlt");
	}
}

void kmain(struct mboot *mboot, unsigned int magic)
{
	struct thread *thread;
	uint32 esp;
	uint32 eflags;

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
	//timer_init();
		asm("movl %%esp, %0" : "=a"(esp));
		screen_putstr(kprintf(buf, "ORIG kstack! %x | ", esp));
	interrupts_start();
	//mboot_load_modules(mboot);

	thread = thread_create(proc_create_kernel_proc(), (uint32)dumper, THREAD_KERNEL);
	screen_putstr(kprintf(buf, "kthread: %x kstack:%x stack:%x! esp:%x\n", thread, thread->kernel_stack, thread->stack, thread->state.esp));
	sched_thread_ready(thread);
	thread = thread_create(proc_create_kernel_proc(), (uint32)dumper, THREAD_KERNEL);
	screen_putstr(kprintf(buf, "kthread: %x kstack:%x stack:%x! esp:%x\n", thread, thread->kernel_stack, thread->stack, thread->state.esp));
	sched_thread_ready(thread);

/*
	mpc = mp_find()->config;
	
	ptr = (uint8*)mpc + sizeof(*mpc);
	for (i = 0; i < mpc->entry_count; ++i) {
		if (*ptr == 0){
			struct MP_proc_entry *pe_ptr = (struct MP_proc_entry*)ptr;
			screen_putstr(kprintf(buf, "lapic id:%i ", pe_ptr->lapic_id));
			screen_putstr(kprintf(buf, "cpu_flags:%x\n", pe_ptr->cpu_flags));
			ptr += 20;
		}else
			break;
	}
*/
	while(1)
		asm("hlt");
}
