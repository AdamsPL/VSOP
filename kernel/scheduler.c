#include "scheduler.h"

#include "config.h"
#include "cpu.h"
#include "screen.h"
#include "util.h"
#include "memory.h"
#include "interrupts.h"
#include "locks.h"
#include "timer.h"
#include "paging.h"
#include "gdt.h"
#include "list.h"

struct scheduler
{
	struct list prio_queues[MAX_PRIORITY];
	struct thread *idle_thread;
	struct thread *current_thread;
	struct thread *next_thread;
};

static struct scheduler schedulers[MAX_CPU];

static void idle_loop(void)
{
	while(1)
		asm("hlt");
}

static void scheduler_add(struct scheduler *this, struct thread *thread)
{
	list_push(this->prio_queues + thread->priority, thread);
}

static void scheduler_init(struct scheduler *this)
{
	kmemset((uint8*)this, 0, sizeof(*this));
	this->idle_thread = thread_create(proc_get_by_pid(0), (uint32)idle_loop, THREAD_KERNEL);
	this->current_thread = this->idle_thread;
}

static void scheduler_put_back(struct scheduler *this, struct thread *th)
{
	if (!th)
		return;

	th->priority = truncate(th->priority + 1, 0, MAX_PRIORITY - 1);
	list_push(this->prio_queues + th->priority, th);
}

static void scheduler_move_up(struct scheduler *this)
{
	int p;
	struct thread *t;
	
	for (p = MAX_PRIORITY - 1; p > 0; --p)
	{
		t = list_pop(this->prio_queues + p);
		list_push(this->prio_queues + p - 1, t);
	}
}

static void scheduler_pick(struct scheduler *this)
{
	int p;
	struct thread *t;

	for (p = 0; p < MAX_PRIORITY; ++p)
	{
		t = list_pop(this->prio_queues + p);
		if (t)
		{
			this->next_thread = t;
			return;
		}
	}
	this->next_thread = this->idle_thread;
}

void _task_switch(void)
{
}


static void scheduler_switch(struct scheduler *this)
{
	struct thread *from_thread = this->current_thread;
	struct thread *to_thread = this->next_thread;

	if (!to_thread)
		return;

	if (from_thread->parent->pdir != to_thread->parent->pdir)
		page_dir_switch(to_thread->parent->pdir);

	tss_set_stack(cpuid(), to_thread->kernel_stack);

	this->current_thread = to_thread;
	this->next_thread = 0;

	asm volatile(\
			"pushl %%esi\n" 		\
			"pushl %%edi\n" 		\
			"pushl %%ebp\n" 		\
			"movl %%esp, %0\n" 		\
			"movl %2, %%esp\n" 		\
			"movl $1f, %1\n" 		\
			"pushl %3\n" 			\
			"jmp _task_switch\n" 	\
			"1:\n" 					\
			"popl %%ebp\n" 			\
			"popl %%edi\n" 			\
			"popl %%esi\n" 			\
			: 						\
		   	"=m" (from_thread->esp),\
			"=m" (from_thread->eip) \
			: 						\
			"m" (to_thread->esp), 	\
			"m" (to_thread->eip) 	\
			);
	
}

static int scheduler_get_load(struct scheduler *this)
{
	int p;
	int result = 0;

	for (p = 0; p < MAX_PRIORITY; ++p)
		result += list_size(this->prio_queues + p);

	return result;
}

static struct scheduler *scheduler_find()
{
	int lowest = 0xFFFFFF;
	struct scheduler *sched = schedulers;
	int tmp;
	int i;

	for (i = 1; i < cpu_count(); ++i)
	{
		tmp = scheduler_get_load(schedulers + i);
		if (tmp < lowest)
		{
			sched = schedulers + i;
			lowest = tmp;
		}
	}
	return sched;
}

void sched_thread_ready(struct thread *thread)
{
	struct scheduler *sched = scheduler_find();

	scheduler_add(sched, thread);
}

uint8 sched_can_run(struct scheduler *this)
{
	if (!this->next_thread->event)
		return 1;
	if (this->next_thread->event(this->next_thread))
	{
		this->next_thread->event = 0;
		return 1;
	}
	scheduler_put_back(this, this->next_thread);
	return 0;
}

uint8 sched_tick(struct thread_state *state)
{
	struct scheduler *sched = schedulers + cpuid();

	scheduler_put_back(sched, sched->current_thread);
	while(1)
	{
		scheduler_move_up(sched);
		scheduler_pick(sched);
		if (sched_can_run(sched))
			break;
	}
	scheduler_switch(sched);

	return INT_OK;
}

void sched_init()
{
	int i;
	
	for (i = 0; i < cpu_count(); ++i)
		scheduler_init(schedulers + i);
}

static void sched_thread_wait(struct thread *thread, thread_event event)
{
	thread->event = event;
	sched_yield();
}

void sched_thread_sleep(uint64 ticks)
{
	struct thread *cur = sched_cur_thread();
	cur->wait_timer = timer_get_ticks() + ticks;
	sched_thread_wait(cur, thread_timer_event);
}

void sched_thread_wait_for_msg()
{
	struct thread *cur = sched_cur_thread();
	sched_thread_wait(cur, thread_msg_event);
}

void sched_yield(void)
{
	sched_tick(0);
}

struct thread *sched_cur_thread(void)
{
	struct scheduler *sched = schedulers + cpuid();
	return sched->current_thread;
}

void sched_start_timer()
{
	lapic_set(0x320, 0x00020080);
	lapic_set(0x3E0, 0xB);
	lapic_set(0x380, 0x00100000);

	while(1)
		asm("hlt");
}

struct process *sched_cur_proc(void)
{
	return sched_cur_thread()->parent;
}
