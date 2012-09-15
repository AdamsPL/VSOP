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
	struct task *idle_task;
};

struct scheduler schedulers[MAX_CPU];
struct task_list ready_list;

static struct task *task_create(struct thread *thread)
{
	char buf[128];
	struct task *ptr = NEW(struct task);
	ptr->thread = thread;
	screen_putstr(kprintf(buf, "NEW task:%x thread:%x!\n", ptr, thread));
	return ptr;
}

static void task_delete(struct task *task)
{
	DELETE(task);
}

static struct task *task_list_pop(struct task_list *list)
{
	struct task *ptr;

	if (!list || !list->head)
		return 0;

	ptr = list->head;

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
	struct thread *idle_thread = NEW(struct thread);
	struct task *idle_task = task_create(idle_thread);
	idle_thread->parent = proc_create_kernel_proc();
	this->current_task = idle_task;
	this->idle_task = idle_task;
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

void _task_switch(void)
{
}

static void sched_switch(struct thread_state *context, struct task *to_task)
{
	struct scheduler *this = sched_current();
	struct thread *from_thread = this->current_task->thread;
	struct thread *to_thread = to_task->thread;

	char buf[128];

	if (this->current_task == to_task)
		return;

	//screen_putstr(kprintf(buf, "from_task: %x to_task: %x\n", this->current_task, to_task));
	//screen_putstr(kprintf(buf, "from_thread: %x to_thread: %x\n", from_thread, to_thread));
	//screen_putstr(kprintf(buf, "from_esp: %x to_esp: %x\n", from_thread->esp, to_thread->esp));
	//screen_putstr(kprintf(buf, "from_parent: %x to_parent: %x\n", from_thread->parent, to_thread->parent));

	page_dir_switch(to_task->thread->parent->pdir);
	tss_set_stack(cpuid(), to_task->thread->kernel_stack);
	this->current_task = to_task;

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

static uint8 sched_tick(struct thread_state *state)
{
	struct scheduler *this = sched_current();
	char buf[128];
	struct task_list *list = 0;
	struct task *task = 0;
	int prio = 0;

	screen_putstr(kprintf(buf, "cur: %x idle:%x!\n", this->current_task, this->idle_task));
	if (this->current_task != this->idle_task)
	{
		//screen_putstr(kprintf(buf, "putting back task:%x to queue: %i\n", this->current_task, this->current_prio));
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
	//section_enter(ready_list.lock);
	if (ready_list.head && ready_list.head->thread->priority < prio)
	{
		//screen_putstr(kprintf(buf, "picking from ready list!\n"));
		prio = ready_list.head->thread->priority;
		list = &ready_list;
	}

	if (!list)
		return INT_OK;
	//screen_putstr(kprintf(buf, "list OK!\n"));

	task = task_list_pop(list);
	//screen_putstr(kprintf(buf, "pop OK!\n"));
	this->current_prio = prio;
	screen_putstr(kprintf(buf, "%x to %x! esp:%x prio:%x\n", this->current_task, task, esp(), this->current_prio));
	sched_switch(state, task);
	//screen_putstr(kprintf(buf, "sched: ready to go!\n"));
	//section_leave(ready_list.lock);

	return INT_OK;
}

void sched_thread_ready(struct thread *thread)
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
	lapic_set(0x380, 0xaa000000);

	interrupts_register_handler(INT_SCHED_TICK, sched_tick);
}
