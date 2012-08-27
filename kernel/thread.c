#include "thread.h"
#include "memory.h"
#include "paging.h"

struct thread *thread_create(struct process *parent, uint32 entry)
{
	struct thread *new = NEW(struct thread);
	/*TODO:FIXME*/
	/*When creating a second stack for a second thread we will overwrite the pagedir associated with the first one!!!*/
	uint32 stack = 0xa00000;

	page_dir_switch(parent->pdir);
	paging_map(stack, mem_phys_alloc(), PAGE_USERMODE | PAGE_PRESENT | PAGE_WRITABLE);
	page_dir_switch(KERNEL_PAGE_DIR_PHYS);

	new->parent = parent;
	new->kernel_stack = (uint32)kmalloc(PAGE_SIZE) + PAGE_SIZE;

	regs_init(&new->state, stack, entry);
	new->stack = new->state.esp;

	return new;
}

void thread_restore_state(struct thread *this, struct thread_state *state)
{
	if (!this || !state)
		return;
	kmemcpy((uint8*)state, (uint8*)&this->state, sizeof(*state));
}

void thread_save_state(struct thread *this, struct thread_state *state)
{
	if (!this || !state)
		return;
	kmemcpy((uint8*)&this->state, (uint8*)state, sizeof(*state));
}

void thread_wait(struct thread *this, uint64 time)
{
	this->wait_time = time;
}
