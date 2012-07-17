#include "locks.h"

void section_enter(uint32 *lock)
{
	while(__sync_lock_test_and_set(lock, 1));
}

void section_leave(uint32 *lock)
{
	__sync_lock_release(lock);
}
