#include "syscall.h"

#include "screen.h"
#include "util.h"
#include "ipc.h"
#include "paging.h"
#include "drivers.h"

void syscall(struct thread_state *state)
{
	char buf[128];
	uint32 id = state->eax;
	int result;

	switch (id)
	{
		case SYS_CALL_EXIT:
			/*
			screen_putstr(kprintf(buf, "exit syscall!\n"));
			*/
			while(1);
			break;
		case SYS_CALL_CONNECT:
			/*screen_putstr(kprintf(buf, "connect syscall!\n"));*/
			state->eax = ipc_connect(proc_cur(), state->ebx);
			break;
		case SYS_CALL_SEND:
			/*screen_putstr(kprintf(buf, "send syscall!\n"));*/
			state->eax = ipc_send(proc_get_descr(proc_cur(), state->ebx).send_to, (uint8 *)(state->ecx), state->edx);
			break;
		case SYS_CALL_RECEIVE:
			result = proc_find_queue(proc_cur());
			*((int*)state->ebx) = result;
			if (result == -1){
				state->eax = 0;
			}else{
				state->eax = ipc_receive(proc_get_descr(proc_cur(), result).rec_from, (uint8 *)(state->ecx), state->edx);
			}
			break;
		case SYS_CALL_WAIT:
			/*screen_putstr(kprintf(buf, "wait syscall!\n"));*/
			break;
		case SYS_CALL_MMAP:
			/*screen_putstr(kprintf(buf, "mmap syscall!\n"));*/
			paging_map(0x2000, state->ebx, PAGE_USERMODE | PAGE_WRITABLE | PAGE_PRESENT);
			state->eax = 0x2000;
			break;
		case SYS_CALL_REGISTER:
			/*screen_putstr(kprintf(buf, "register syscall!\n"));*/
			state->eax = server_set(proc_cur(), (char*)state->ebx, state->ecx);
			break;
		case SYS_CALL_QUERY:
			/*screen_putstr(kprintf(buf, "query syscall!\n"));*/
			state->eax = server_get((char*)state->ebx);
			break;
		default:
			screen_putstr(kprintf(buf, "Unknown syscall!\n"));
			state->eax = 0xDEADC0DE;
	}
}
