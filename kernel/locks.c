#include "locks.h"
#include "util.h"
#include "scheduler.h"
#include "screen.h"
#include "cpu.h"

void section_enter(lock_t *lock)
{
	while(!__sync_bool_compare_and_swap(lock, 0, 1))
		while(*lock)
			;
}

void section_leave(lock_t *lock)
{
	/*
	char buf[128];
	*/
	*lock = 0;
	/*
	screen_putstr(kprintf(buf, "thread: UNLOCK:%x val:%x\n", lock, *lock));
	*/
	
}
