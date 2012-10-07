#ifndef _THREAD_H
#define _THREAD_H

#include "process.h"
#include "interrupts.h"
#include "locks.h"

enum thread_status
{
	THREAD_READY,
	THREAD_WAITING,
	THREAD_QUEUED,
	THREAD_EXECUTING
};


struct thread
{
	struct process *parent;
	int priority;
	uint32 stack;
	uint32 kernel_stack;
	uint64 wait_time;
	int msg_descr;
	uint32 esp;
	uint32 eip;
	uint32 sched_exec_ticks;
	uint32 sched_wait_ticks;
	enum thread_status state;
};

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags);

#endif
