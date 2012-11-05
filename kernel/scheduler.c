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
#include "screen.h"

struct scheduler
{
	struct list prio_queues[MAX_PRIORITY];
	struct thread *idle_thread;
	struct thread *current_thread;
	struct thread *next_thread;
	struct list waiting_threads;
	int load;
	uint8 started;
};

static struct scheduler schedulers[MAX_CPU];

void sched_idle_loop(void)
{
	while(1)
		asm("hlt");
}

static void scheduler_put_back(struct scheduler *this, struct thread *th)
{
	/*
	char buf[128];

	screen_putstr(kprintf(buf, "Putting back %x, idle:%x\n", th, this->idle_thread));
	*/
	if (!th)
		return;

	if (th == this->idle_thread)
		return;

	/*th->priority = truncate(th->priority + 1, 0, MAX_PRIORITY - 1);*/
	list_push(this->prio_queues + th->priority, th);
}
/*
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
*/
static struct thread *scheduler_pick(struct scheduler *this)
{
	int p;
	struct thread *t;

	for (p = 0; p < MAX_PRIORITY; ++p)
	{
		t = list_pop(this->prio_queues + p);
		if (t)
			return t;
	}
	return 0;
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

	if (from_thread == to_thread)
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

static struct scheduler *scheduler_find()
{
	struct scheduler *sched = schedulers;
	int lowest = sched->load;
	int tmp;
	int i;

	for (i = 1; i < MAX_CPU; ++i)
	{
		if (!schedulers[i].started)
			continue;
		tmp = schedulers[i].load;
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
	char buf[128];
	struct scheduler *sched = scheduler_find();
	int id = ((int)sched - (int)schedulers) / sizeof(*sched);
	screen_putstr(kprintf(buf, "Adding thread:%x to sched:[%x]\n", thread, id));
	list_push(sched->prio_queues + thread->priority, thread);
	sched->load++;
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
	return 0;
}

static void scheduler_wake_all(struct scheduler *this)
{
	struct thread *th = (struct thread *)list_pop(&this->waiting_threads);
	while(th)
	{
		scheduler_put_back(this, th);
		th = (struct thread *)list_pop(&this->waiting_threads);
	}
}

uint8 sched_tick(struct thread_state *state)
{
	struct scheduler *sched = schedulers + cpuid();

	scheduler_put_back(sched, sched->current_thread);
	scheduler_wake_all(sched);

	while(1)
	{
		/*scheduler_move_up(sched);*/
		sched->next_thread = scheduler_pick(sched);
		if (!sched->next_thread)
			sched->next_thread = sched->idle_thread;
		if (sched_can_run(sched))
			break;
#ifdef CONF_PREEMPTIBLE
		scheduler_put_back(sched, sched->next_thread);
#else
		list_push(&sched->waiting_threads, sched->next_thread);
#endif
	}
	scheduler_switch(sched);

	return INT_OK;
}

static void scheduler_init(struct scheduler *this)
{
	kmemset((uint8*)this, 0, sizeof(*this));
	this->idle_thread = thread_create(proc_get_by_pid(0), (uint32)sched_idle_loop, THREAD_KERNEL);
	this->current_thread = this->idle_thread;
	this->started = 0;
}

void scheduling_init()
{
	int i;
	interrupts_register_handler(INT_SCHED_TICK, sched_tick);
	for (i = 0; i < MAX_CPU; ++i)
		scheduler_init(schedulers + i);
}

void sched_thread_wait(struct thread *thread, thread_event event)
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
}

struct process *sched_cur_proc(void)
{
	return sched_cur_thread()->parent;
}

void sched_yield(void)
{
	/*
	lapic_set(LAPIC_TPR, INT_SCHED_TICK);
	sched_tick(0);
	*/
}

void sched_ready()
{
	struct scheduler *this = schedulers + cpuid();
	this->started = 1;
}
