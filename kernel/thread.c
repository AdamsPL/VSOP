#include "thread.h"
#include "memory.h"
#include "paging.h"
#include "palloc.h"
#include "screen.h"

void _leave_kernel(void);

static void alloc_stack(uint32 *stack, uint32 size)
{
	*stack -= size;
}

static void push_stack(uint32 *stack, uint32 value)
{
	uint32 **ptr;
	*stack -= sizeof(value);
	ptr = (uint32**)stack;
	**ptr = value;
}

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags)
{
	/*TODO:FIXME*/
	/*When creating a second stack for a second thread we will overwrite the pagedir associated with the first one!!!*/
	uint32 stack = 0;
	struct thread *new = NEW(struct thread);

	new->parent = parent;
	new->kernel_stack = (uint32)kmalloc(PAGE_SIZE) + PAGE_SIZE - 16;

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
	new->esp = new->kernel_stack;

	alloc_stack(&new->esp, sizeof(struct thread_state));
	regs_init((struct thread_state*)new->esp, stack + PAGE_SIZE - 16, entry, flags);

	push_stack(&new->esp, 0);/*esi*/
	push_stack(&new->esp, 0);/*edi*/
	push_stack(&new->esp, new->kernel_stack);/*ebp*/

	new->stack = stack;

	return new;
}
