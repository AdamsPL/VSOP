#include "locks.h"
#include "util.h"
#include "scheduler.h"

void section_enter(lock_t *lock)
{
	interrupts_disable();
	while(__sync_lock_test_and_set(lock, 1))
		;
}

void section_leave(lock_t *lock)
{
	__sync_lock_release(lock);
	interrupts_enable();
}
