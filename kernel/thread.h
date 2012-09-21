#ifndef _THREAD_H
#define _THREAD_H

#include "process.h"
#include "interrupts.h"

struct thread
{
	struct process *parent;
	int priority;
	uint32 stack;
	uint32 kernel_stack;
	uint64 wait_time;
	uint32 esp;
	uint32 eip;
	uint32 sched_exec_ticks;
	uint32 sched_wait_ticks;
	struct thread *next;
};

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags);
void thread_wait(struct thread *this, uint64 time);

#endif
