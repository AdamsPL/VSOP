#ifndef _THREAD_H
#define _THREAD_H

#include "process.h"
#include "interrupts.h"

struct thread
{
	struct process *parent;
	uint32 priority;
	uint32 stack;
	uint32 kernel_stack;
	struct thread *next;
	uint64 wait_time;
	uint32 esp;
	uint32 eip;
};

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags);
void thread_wait(struct thread *this, uint64 time);

#endif
