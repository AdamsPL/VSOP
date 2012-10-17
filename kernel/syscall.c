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
	/*screen_putstr(kprintf(buf, "syscall enter: %x proc: %x!\n", id, cur_proc->pid));*/
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
			/*
			screen_putstr(kprintf(buf, "proc: %x read buf:%x len: %x\n", cur_proc, state->ebx, state->ecx));
			*/
			msg = proc_recv(cur_proc);
			/*
			screen_putstr(kprintf(buf, "read buf:%x len: %x\n", state->ebx, state->ecx));
			*/
			kmemcpy((uint8*)state->ebx, (uint8*)msg->buf, state->ecx);
			message_free(msg);
			state->eax = 0;
			break;
		case SYSCALL_WRITE:
			target_proc = proc_get_by_pid(state->ebx);
			if (!target_proc)
				break;
			/*
			screen_putstr(kprintf(buf, "write pid:%x buf:%x len: %x\n", state->ebx, state->ecx, state->edx));
			*/
			msg = message_alloc(state->edx, (uint8*)state->ecx);
			proc_send(msg, target_proc);
			state->eax = 0;
			break;
		case SYSCALL_MMAP:
			paging_map(state->ebx, state->ecx, PAGE_USERMODE | PAGE_WRITABLE | PAGE_PRESENT);
			break;
		default:
			break;
	}
	/*
	screen_putstr(kprintf(buf, "syscall leave: %x proc: %x! result: %x\n", id, cur_proc->pid, state->eax));
	*/
	return INT_OK;
}

void syscalls_init(void)
{
	interrupts_register_handler(INT_SYSCALL, syscall);
}
