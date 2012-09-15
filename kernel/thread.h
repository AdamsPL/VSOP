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
	struct thread_state state;
};

struct thread *thread_create(struct process *parent, uint32 entry, enum thread_flags flags);
void thread_restore_state(struct thread *this, struct thread_state *state);
void thread_save_state(struct thread *this, struct thread_state *state);
void thread_wait(struct thread *this, uint64 time);

#endif
