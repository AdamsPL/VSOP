#include "syscall.h"

#include "scheduler.h"
#include "screen.h"
#include "util.h"
#include "paging.h"
#include "drivers.h"
#include "process.h"
#include "cpu.h"
#include "message.h"

uint8 syscall(struct thread_state *state)
{
	char buf[128];
	uint32 id = state->eax;
	int result = -1;
	struct process *target_proc;
	struct process *cur_proc = sched_cur_proc();
	struct message *msg;

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
		case SYSCALL_PIDOF:
			target_proc = proc_get_by_name((char*)state->ebx);
			if (target_proc)
				result = target_proc->pid;
			else
				result = -1;
			state->eax = result;
			break;
		case SYSCALL_READ:
			msg = proc_recv(cur_proc);
			kmemcpy((uint8*)state->ebx, (uint8*)msg->buf, state->ecx);
			message_free(msg);
			state->eax = 0;
			break;
		case SYSCALL_WRITE:
			target_proc = proc_get_by_pid(state->ebx);
			if (!target_proc)
				break;
			msg = message_alloc(state->edx, (uint8*)state->ecx);
			proc_send(msg, target_proc);
			state->eax = 0;
			break;
		case SYSCALL_MMAP:
			paging_map(state->ebx, state->ecx, PAGE_USERMODE | PAGE_WRITABLE | PAGE_PRESENT);
			break;
		case SYSCALL_HANDLE:
			driver_register(cur_proc->pid, state->ebx);
			break;
		case SYSCALL_PEEK:
			result = proc_peek(cur_proc);
			state->eax = result;
			break;

	}
	return INT_OK;
}

void syscalls_init(void)
{
	interrupts_register_handler(INT_SYSCALL, syscall);
}
