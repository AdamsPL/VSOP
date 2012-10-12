#include "syscall.h"

#include "scheduler.h"
#include "screen.h"
#include "util.h"
#include "ipc.h"
#include "paging.h"
#include "drivers.h"
#include "process.h"

uint8 syscall(struct thread_state *state)
{
	/*char buf[128];*/
	uint32 id = state->eax;
	int result = -1;
	struct process *cur_proc = proc_get_by_pid(sched_cur_proc());
	struct process *target_proc;

	/*screen_putstr(kprintf(buf, "syscall enter: %x proc: %x!\n", id, cur_proc->pid));*/
	switch (id)
	{
		case SYSCALL_EXIT:
			/*
			screen_putstr(kprintf(buf, "exit syscall: %x\n", state->ebx));
			*/
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
			target_proc = proc_get_by_name((char*)state->ebx);
			result = -1;
			if (target_proc)
				result = ipc_connect(cur_proc->pid, target_proc->pid);
			state->eax = result;
			break;
		case SYSCALL_SELECT:
			result = sched_thread_select_msg();
			state->eax = result;
			break;
		case SYSCALL_READ:
			/*screen_putstr(kprintf(buf, "%x read(%x, %x, %x)!\n", cur_proc->pid, state->ebx, state->ecx, state->edx));*/
			result = ipc_receive(proc_get_descr(cur_proc, state->ebx), (uint8*)state->ecx, state->edx); 
			state->eax = result;
			break;
		case SYSCALL_WRITE:
			/*screen_putstr(kprintf(buf, "%x write(%x, %x, %x)!\n", cur_proc->pid, state->ebx, state->ecx, state->edx));*/
			result = ipc_send(proc_get_descr(cur_proc, state->ebx), (uint8*)state->ecx, state->edx); 
			state->eax = result;
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
