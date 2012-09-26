#include "locks.h"

void section_enter(lock_t *lock)
{
	while(__sync_lock_test_and_set(lock, 1))
		while(*lock);
}

void section_leave(lock_t *lock)
{
	__sync_lock_release(lock);
}
