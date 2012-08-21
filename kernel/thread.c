#include "thread.h"
#include "memory.h"
#include "paging.h"

struct thread *thread_create(struct process *parent, uint32 entry)
{
	struct thread *new = NEW(struct thread);
	uint32 stack = 0xa00000;

	paging_map(stack, mem_phys_alloc(), PAGE_USERMODE | PAGE_PRESENT | PAGE_WRITABLE);

	new->parent = parent;
	new->kernel_stack = (uint32)kmalloc(PAGE_SIZE) + PAGE_SIZE;

	stack += PAGE_SIZE;

	regs_init(&new->state, stack, entry);

	return new;
}

void thread_restore_state(struct thread *this, struct thread_state *state)
{
	kmemcpy((uint8*)state, (uint8*)&this->state, sizeof(*state));
}

void thread_save_state(struct thread *this, struct thread_state *state)
{
	kmemcpy((uint8*)&this->state, (uint8*)state, sizeof(*state));
}
