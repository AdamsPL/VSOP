#include "scheduler.h"

#include "config.h"
#include "cpu.h"
#include "screen.h"
#include "util.h"
#include "multitasking.h"
#include "memory.h"
#include "interrupts.h"

#define MAX_PRIORITY 8
#define MAX_TASKS_PER_PRIO 32

struct task
{
	struct thread *thread;
	struct task *next;
};

struct task_list
{
	struct task *head;
	struct task *tail;
	lock_t lock;
};

struct scheduler
{
	struct task *current_task;
	uint8 current_prio;
	struct task_list prio_queues[MAX_PRIORITIES];
};

struct scheduler schedulers[MAX_CPU];
struct task_list ready_list;

static struct task *task_create(struct thread *thread)
{
	struct task *ptr = NEW(struct task);
	ptr->thread = thread;
	return ptr;
}

static void task_delete(struct task *task)
{
	DELETE(task);
}

static struct task *task_list_pop(struct task_list *list)
{
	struct task *ptr = list->head;

	if (!ptr)
		return 0;

	if (list->head == list->tail)
		list->tail = 0;

	list->head = ptr->next;
	ptr->next = 0;

	return ptr;
}

static void task_list_push(struct task_list *list, struct task *task)
{
	if (!list->tail)
	{
		list->tail = task;
		list->head = task;
		return;
	}

	list->tail->next = task;
	list->tail = task;
}

static void init(struct scheduler *this)
{
	ZEROS(this);
}


struct scheduler *sched_current()
{
	return schedulers + cpuid();
}

static uint8 get_lower_prio(uint8 prio)
{
	++prio;
	if (prio >= MAX_PRIORITY)
		prio = MAX_PRIORITY - 1;
	return prio;
}

static void sched_switch(struct thread_state *context, struct task *to_task)
{
	struct scheduler *this = sched_current();

	regs_print(context);
	regs_print(&to_task->thread->state);

	this->current_task = to_task;
	thread_restore_state(to_task->thread, context);
	page_dir_switch(to_task->thread->parent->pdir);
}

static uint8 sched_tick(struct thread_state *state)
{
	struct scheduler *this = sched_current();
	char buf[128];
	struct task_list *list = 0;
	struct task *task = 0;
	int prio = 0;

	screen_putstr(kprintf(buf, "%x tick!\n", this));

	if (this->current_task)
	{
		task_list_push(this->prio_queues + get_lower_prio(this->current_prio), this->current_task);
	}
	for (prio = 0; prio < MAX_PRIORITY; ++prio)
	{
		if (this->prio_queues[prio].head)
		{
			list = this->prio_queues + prio;
			break;
		}
	}
	screen_putstr(kprintf(buf, "%x tock!\n", this));
	//section_enter(ready_list.lock);
	if (ready_list.head && ready_list.head->thread->priority < prio)
	{
		prio = ready_list.head->thread->priority;
		list = &ready_list;
	}
	task = task_list_pop(list);
	this->current_prio = prio;
	//section_leave(ready_list.lock);

	screen_putstr(kprintf(buf, "sched: switching from %x to %x!\n", this->current_task, task));
	sched_switch(state, task);
	return INT_OK;
}

void shed_thread_ready(struct thread *thread)
{
	struct task *task = task_create(thread);
	task_list_push(&ready_list, task);
}

struct thread *sched_current_thread()
{
	struct scheduler *this = sched_current();
	if (!this->current_task)
		return 0;
	return this->current_task->thread;
}

void sched_init_all()
{
	int i;
	for (i = 0; i < MAX_CPU; ++i)
		init(schedulers + i);

	lapic_set(0x320, 0x20080);
	lapic_set(0x3E0, 0xB);
	lapic_set(0x380, 0x43000000);

	interrupts_register_handler(INT_SCHED_TICK, sched_tick);
}
