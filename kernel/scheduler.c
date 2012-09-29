#include "scheduler.h"

#include "config.h"
#include "cpu.h"
#include "screen.h"
#include "util.h"
#include "multitasking.h"
#include "memory.h"
#include "interrupts.h"

#define MAX_PRIORITY 8

static lock_t lock = 0;

struct thread_list
{
	struct thread *head;
	struct thread *tail;
	lock_t lock;
};

struct thread_list prio_queues[MAX_PRIORITY];
struct thread *idle_thread[MAX_CPU];
struct thread *current_thread[MAX_CPU];

static void sched_print()
{
	struct thread *thread;
	int pri, i;
	char buf[128];

	for (i = 0; i < cpu_count(); ++i)
	{
		screen_putstr(kprintf(buf, "cpu: %i cur: %x is_idle: %i\n", i, current_thread[i], current_thread[i] == idle_thread[i]));
	}

	return;

	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		screen_putstr(kprintf(buf, "[%i h:%x t:%x]: ", pri, prio_queues[pri].head, prio_queues[pri].tail));
		thread = prio_queues[pri].head;
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

static void init()
{
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
	section_leave(&lock);
}

static sched_put_back()
{
	struct thread *thread = current_thread[cpuid()];
	if (thread == idle_thread[cpuid()])
		return;
	++thread->sched_exec_ticks;
	thread->sched_wait_ticks = 0;
	if (thread->wait_time > 0)
		timer_manage_thread(thread);
	else
		thread_list_push(prio_queues + thread->priority, thread);
}

static sched_move_down()
{
	struct thread *thread;
	int pri;
	char buf[128];
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = prio_queues[pri].head;
		if (!thread)
			continue;
		if (thread->sched_exec_ticks >= (1 << thread->priority))
		{
			//screen_putstr(kprintf(buf, "down %x prio: %i -> %i\n", thread, thread->priority, get_lower_prio(thread->priority)));
			thread->sched_exec_ticks = 0;
			thread->sched_wait_ticks = 0;
			thread->priority = get_lower_prio(thread->priority);
			thread_list_pop(prio_queues + pri);
			thread_list_push(prio_queues + thread->priority, thread);
		}
	}
}

static sched_move_up()
{
	struct thread *thread;
	int pri;
	char buf[128];
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = prio_queues[pri].head;
		if (!thread)
			continue;
		++thread->sched_wait_ticks;
		if (thread->sched_wait_ticks > (1 << thread->priority))
		{
				//screen_putstr(kprintf(buf, "up %x prio: %i -> %i\n", thread, thread->priority, get_higher_prio(thread->priority)));
			thread->sched_exec_ticks = 0;
			thread->sched_wait_ticks = 0;
			thread->priority = get_higher_prio(thread->priority);
			thread_list_pop(prio_queues + pri);
			thread_list_push(prio_queues + thread->priority, thread);
		}
	}
}

static sched_switch(struct thread *from_thread, struct thread *to_thread)
{
	char buf[128];
	current_thread[cpuid()] = to_thread;

	if (from_thread == to_thread)
		return;

	if (from_thread->parent->pdir != to_thread->parent->pdir)
		page_dir_switch(to_thread->parent->pdir);

	tss_set_stack(cpuid(), to_thread->kernel_stack);
	current_thread[cpuid()] = to_thread;

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

static sched_pick()
{
	struct thread *thread;
	int pri;
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = thread_list_pop(prio_queues + pri);
		if (!thread)
			continue;
		sched_switch(current_thread[cpuid()], thread);
		return;
	}
	sched_switch(current_thread[cpuid()], idle_thread[cpuid()]);
}


static uint8 sched_tick(struct thread_state *state)
{
	section_enter(&lock);

	//sched_print();

	sched_put_back();
	sched_move_down();
	sched_move_up();
	sched_pick();

	section_leave(&lock);

	return INT_OK;
}

void sched_thread_ready(struct thread *thread)
{
	section_enter(&lock);
	thread_list_push(prio_queues + thread->priority, thread);
	section_leave(&lock);
}

void sched_init()
{
	section_leave(&lock);

	struct thread *new_thread = thread_create(proc_get_by_pid(0), (uint32)idle_loop, THREAD_KERNEL);

	current_thread[cpuid()] = new_thread;
	idle_thread[cpuid()] = new_thread;

	interrupts_register_handler(INT_SCHED_TICK, sched_tick);

	lapic_set(0x320, 0x20080);
	lapic_set(0x3E0, 0xB);
	lapic_set(0x380, 0x05000000);

	section_leave(&lock);
}

void sched_thread_sleep(uint64 ticks)
{
	current_thread[cpuid()]->wait_time += ticks;
	asm("int $0x80");
}

pid_t sched_cur_proc()
{
	return current_thread[cpuid()]->parent->pid;
}
