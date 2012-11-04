#include "syscall.h"

#include "scheduler.h"
#include "screen.h"
#include "util.h"
#include "paging.h"
#include "drivers.h"
#include "process.h"
#include "cpu.h"
#include "timer.h"
#include "stream.h"

uint8 syscall(struct thread_state *state)
{
	char buf[128];
	uint32 id = state->eax;
	int result = -1;
	struct process *target_proc;
	struct process *cur_proc = sched_cur_proc();
	struct thread *cur_thread = sched_cur_thread();

	switch (id)
	{
		case SYSCALL_EXIT:
			screen_putstr(kprintf(buf, "proc:%x thread: %x cpu:%x exit syscall: %x\n", sched_cur_proc(), sched_cur_thread(), cpuid(), state->ebx));
			break;
		case SYSCALL_WAIT:
			if (state->ebx > 0)
				sched_thread_sleep(state->ebx);
			break;
		case SYSCALL_REGISTER:
			result = proc_register(cur_proc, (char*)state->ebx);
			state->eax = result;
			break;
		case SYSCALL_CONNECT:
			/*
			screen_putstr(kprintf(buf, "%s: CONNECT\n", cur_proc->name));
			*/
			target_proc = proc_get_by_name((char*)state->ebx);
			if (target_proc)
			{
				struct stream *input = stream_new();
				struct stream *output = stream_new();
				iostream_attach(&target_proc->iodescr, output, input);
				result = iostream_attach(&cur_proc->iodescr, input, output);
			}
			else
				result = -1;
			break;
		case SYSCALL_SELECT:
			/*
			screen_putstr(kprintf(buf, "%s: SELECT\n", cur_proc->name));
			*/
			while(1)
			{
				result = iostream_select(&cur_proc->iodescr);
				if (result != -1)
					break;
				cur_thread->descr = 0;
				cur_thread->event = stream_select_event;
				sched_yield();
			}
			break;
		case SYSCALL_READ:
			/*
			screen_putstr(kprintf(buf, "%s: READ\n", cur_proc->name));
			*/
			stream_read(cur_proc->iodescr.iostreams[state->ebx].input, (uint8*)state->ecx, state->edx);
			result = 0;
			break;
		case SYSCALL_WRITE:
			/*
			screen_putstr(kprintf(buf, "%s: WRITE\n", cur_proc->name));
			*/
			stream_write(cur_proc->iodescr.iostreams[state->ebx].output, (uint8*)state->ecx, state->edx);
			result = 0;
			break;
		case SYSCALL_MMAP:
			paging_map(state->ebx, state->ecx, PAGE_USERMODE | PAGE_WRITABLE | PAGE_PRESENT);
			break;
		case SYSCALL_HANDLE:
			result = driver_register(cur_proc, state->ebx);
			break;
		case SYSCALL_PEEK:
			result = stream_read_length(cur_proc->iodescr.iostreams[state->ebx].input);
			break;
		case SYSCALL_TIME:
			result = timer_get_ticks();
			break;
	}
	state->eax = result;
	return INT_OK;
}

void syscalls_init(void)
{
	interrupts_register_handler(INT_SYSCALL, syscall);
}
