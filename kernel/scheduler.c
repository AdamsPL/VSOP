#include "scheduler.h"

#include "config.h"
#include "cpu.h"
#include "screen.h"
#include "util.h"
#include "multitasking.h"
#include "memory.h"
#include "interrupts.h"

#define MAX_PRIORITY 8


struct thread_list
{
	struct thread *head;
	struct thread *tail;
	lock_t lock;
};

struct scheduler
{
	struct thread_list prio_queues[MAX_PRIORITY];
	struct thread *idle_thread;
	struct thread *current_thread;
};

struct scheduler schedulers[MAX_CPU];

static void sched_print(struct scheduler *this)
{
	struct thread *thread;
	int pri;
	char buf[128];

	screen_putstr(kprintf(buf, "cur: %x\n", this->current_thread));
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		screen_putstr(kprintf(buf, "[%i h:%x t:%x]: ", pri, this->prio_queues[pri].head, this->prio_queues[pri].tail));
		thread = this->prio_queues[pri].head;
		while(thread)
		{
			screen_putstr(kprintf(buf, "%x[->%x], ", thread, thread->next));
			thread = thread->next;
		}
		screen_putstr(kprintf(buf, "\n"));
	}
}

static struct thread *thread_list_pop(struct thread_list *list)
{
	struct thread *ptr;

	if (!list || !list->head)
		return 0;

	ptr = list->head;

	if (list->head == list->tail)
		list->tail = 0;

	list->head = ptr->next;
	ptr->next = 0;

	return ptr;
}

static void thread_list_push(struct thread_list *list, struct thread *thread)
{
	if (!list->tail)
	{
		list->tail = thread;
		list->head = thread;
		return;
	}

	list->tail->next = thread;
	list->tail = thread;
}

static void idle_loop(void)
{
	while(1)
	{
		asm("hlt");
	}
}

static void init(struct scheduler *this)
{
	ZEROS(this);
	struct thread *idle_thread = thread_create(proc_create_kernel_proc(), (uint32)idle_loop, THREAD_KERNEL);
	this->current_thread = idle_thread;
	this->idle_thread = idle_thread;
}

struct scheduler *sched_current()
{
	int cpu = lapic_get(LAPIC_ID) >> 24;
	return schedulers + cpu;
}

static int get_lower_prio(int prio)
{
	++prio;
	return (prio < MAX_PRIORITY) ? prio : MAX_PRIORITY - 1;
}

static int get_higher_prio(int prio)
{
	--prio;
	return (prio > 0) ? prio : 0;
}

void _task_switch(void)
{
}

static sched_put_back(struct scheduler *this)
{
	struct thread *thread = this->current_thread;
	if (thread == this->idle_thread)
		return;
	++thread->sched_exec_ticks;
	thread->sched_wait_ticks = 0;
	thread_list_push(this->prio_queues + thread->priority, thread);
}

static sched_move_down(struct scheduler *this)
{
	struct thread *thread;
	int pri;
	char buf[128];
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = this->prio_queues[pri].head;
		if (!thread)
			continue;
		if (thread->sched_exec_ticks >= (1 << thread->priority))
		{
			//screen_putstr(kprintf(buf, "down %x prio: %i -> %i\n", thread, thread->priority, get_lower_prio(thread->priority)));
			thread->sched_exec_ticks = 0;
			thread->sched_wait_ticks = 0;
			thread->priority = get_lower_prio(thread->priority);
			thread_list_pop(this->prio_queues + pri);
			thread_list_push(this->prio_queues + thread->priority, thread);
		}
	}
}

static sched_move_up(struct scheduler *this)
{
	struct thread *thread;
	int pri;
	char buf[128];
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = this->prio_queues[pri].head;
		if (!thread)
			continue;
		++thread->sched_wait_ticks;
		if (thread->sched_wait_ticks > (1 << thread->priority))
		{
				//screen_putstr(kprintf(buf, "up %x prio: %i -> %i\n", thread, thread->priority, get_higher_prio(thread->priority)));
			thread->sched_exec_ticks = 0;
			thread->sched_wait_ticks = 0;
			thread->priority = get_higher_prio(thread->priority);
			thread_list_pop(this->prio_queues + pri);
			thread_list_push(this->prio_queues + thread->priority, thread);
		}
	}
}

static sched_switch(struct scheduler *this, struct thread *from_thread, struct thread *to_thread)
{
	this->current_thread = to_thread;

	if (from_thread == to_thread)
		return;

	if (from_thread->parent != to_thread->parent)
		page_dir_switch(to_thread->parent->pdir);

	tss_set_stack(cpuid(), to_thread->kernel_stack);
	this->current_thread = to_thread;

	asm volatile(\
			"pushl %%esi\n" 			\
			"pushl %%edi\n" 			\
			"pushl %%ebp\n" 			\
			"movl %%esp, %0\n" 		\
			"movl %2, %%esp\n" 		\
			"movl $1f, %1\n" 			\
			"pushl %3\n" 				\
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

static sched_pick(struct scheduler *this)
{
	struct thread *thread;
	int pri;
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = thread_list_pop(this->prio_queues + pri);
		if (!thread)
			continue;
		sched_switch(this, this->current_thread, thread);
		return;
	}
	sched_switch(this, this->current_thread, this->idle_thread);
}

static uint8 sched_tick(struct thread_state *state)
{
	struct scheduler *this = sched_current();
	char buf[128];

	screen_putstr(kprintf(buf, "sched id: %x\n", lapic_get(LAPIC_ID) >> 24));

	sched_put_back(this);
	sched_move_down(this);
	sched_move_up(this);
	sched_pick(this);

	return INT_OK;
}

void sched_thread_ready(struct thread *thread)
{
	struct scheduler *this = sched_current();
	thread_list_push(this->prio_queues + thread->priority, thread);
}

struct thread *sched_current_thread()
{
	struct scheduler *this = sched_current();
	return this->current_thread;
}

void sched_init()
{
	int cpu = lapic_get(LAPIC_ID) >> 24;
	init(schedulers + cpu);

	interrupts_register_handler(INT_SCHED_TICK, sched_tick);

	lapic_set(0x320, 0x20080);
	lapic_set(0x3E0, 0xB);
	lapic_set(0x380, 0xF6000000);
}
