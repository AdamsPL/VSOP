#ifndef _THREAD_H
#define _THREAD_H

#include "process.h"
#include "interrupts.h"
#include "locks.h"

struct thread;
struct stream;

typedef uint8 (*thread_event)(struct thread *this);

struct thread
{
	struct process *parent;
	int priority;
	uint32 stack;
	uint32 kernel_stack;
	uint32 esp;
	uint32 eip;
	uint64 wait_timer;
	struct stream *descr;
	thread_event event;
};

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags);

#endif
