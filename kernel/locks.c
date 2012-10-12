#include "locks.h"
#include "util.h"
#include "scheduler.h"

void section_enter(lock_t *lock)
{
	while(!__sync_bool_compare_and_swap(lock, 0, 1))
		while(*lock);
}

void section_leave(lock_t *lock)
{
	__sync_lock_release(lock);
}
