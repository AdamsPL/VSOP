#include "thread.h"
#include "memory.h"
#include "paging.h"

void _leave_kernel(void);

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags)
{
	char buf[128];
	struct thread *new = NEW(struct thread);
	/*TODO:FIXME*/
	/*When creating a second stack for a second thread we will overwrite the pagedir associated with the first one!!!*/
	uint32 stack = 0;
	new->parent = parent;
	new->kernel_stack = (uint32)kmalloc(PAGE_SIZE);

	if (flags && THREAD_USERSPACE)
	{
		stack = 0xa00000;
		page_dir_switch(parent->pdir);
		paging_map(stack, mem_phys_alloc(), PAGE_USERMODE | PAGE_PRESENT | PAGE_WRITABLE);
		page_dir_switch(KERNEL_PAGE_DIR_PHYS);
	}
	else
	{
		stack = (uint32)kmalloc(PAGE_SIZE);
	}

	new->eip = (uint32)_leave_kernel;
	new->esp = new->kernel_stack +PAGE_SIZE - sizeof(struct thread_state);

	regs_init((struct thread_state*)new->esp, stack, entry, flags);
	new->stack = stack;

	screen_putstr(kprintf(buf, "kstack: %x\n", new->kernel_stack));


	return new;
}

void thread_wait(struct thread *this, uint64 time)
{
	this->wait_time = time;
}
