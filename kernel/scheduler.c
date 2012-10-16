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

#define MAX_PRIORITY 3

struct list prio_queues[MAX_PRIORITY];
struct list waiting_threads;
struct thread *idle_thread[MAX_CPU];
struct thread *current_thread[MAX_CPU];

static lock_t lock = 0;

static void idle_loop(void)
{
	while(1)
		asm("hlt");
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

static void sched_print()
{
	char buf[128];
	int i;

	for (i = 0; i < MAX_PRIORITY; ++i)
	{
		list_print(prio_queues + i);
	}
	list_print(&waiting_threads);
	screen_putstr(kprintf(buf, "*****\n"));
}

static void sched_put_back(void)
{
	struct thread *thread = sched_cur_thread();
	if (thread == idle_thread[cpuid()])
		return;
	++thread->sched_exec_ticks;
	thread->sched_wait_ticks = 0;

	if (!(thread->state == THREAD_WAITING))
	{
		thread->state = THREAD_QUEUED;
		list_push(prio_queues + thread->priority, thread);
	}
	else
	{
		if (thread->wait_time > 0)
			timer_manage_thread(thread);
	}
}

static void sched_move_down()
{
	struct thread *thread;
	int pri;
	return;
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = (struct thread *)list_peek(prio_queues + pri);
		if (!thread)
			continue;
		if (thread->sched_exec_ticks >= (1 << thread->priority))
		{
			thread->sched_exec_ticks = 0;
			thread->sched_wait_ticks = 0;
			thread->priority = get_lower_prio(thread->priority);
			list_pop(prio_queues + pri);
			list_push(prio_queues + thread->priority, thread);
		}
	}
}

static void sched_move_up()
{
	struct thread *thread;
	int pri;
	return;
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = (struct thread *)list_peek(prio_queues + pri);
		if (!thread)
			continue;
		++thread->sched_wait_ticks;
		if (thread->sched_wait_ticks > (1 << thread->priority))
		{
			thread->sched_exec_ticks = 0;
			thread->sched_wait_ticks = 0;
			thread->priority = get_higher_prio(thread->priority);
			list_pop(prio_queues + pri);
			list_push(prio_queues + thread->priority, thread);
		}
	}
}

static void sched_switch(struct thread *from_thread, struct thread *to_thread)
{
	if (from_thread == to_thread)
	{
		section_leave(&lock);
		return;
	}

	if (from_thread->parent->pdir != to_thread->parent->pdir)
		page_dir_switch(to_thread->parent->pdir);

	tss_set_stack(cpuid(), to_thread->kernel_stack);
	current_thread[cpuid()] = to_thread;

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

static struct thread *sched_pick()
{
	struct thread *thread;
	int pri;
	for (pri = 0; pri < MAX_PRIORITY; ++pri)
	{
		thread = list_pop(prio_queues + pri);
		if (!thread)
			continue;
		thread->state = THREAD_EXECUTING;
		return thread;
	}
	return idle_thread[cpuid()];
}

static void sched_wake_threads(void)
{
	struct thread *thread;

	while ((thread = list_pop(&waiting_threads)))
	{
		if (thread->state == THREAD_WAITING || thread->state == THREAD_READY)
		{
			thread->state = THREAD_QUEUED;
			list_push(prio_queues + thread->priority, thread);
		}
		else
			thread->state = THREAD_READY;
	}
}

uint8 sched_tick(struct thread_state *state)
{
	char buf[128];
	struct thread *next;
	struct thread *prev;
	int i, j, k;

	section_enter(&lock);
	for (i = 0; i < cpu_count(); ++i)
	{
		for (j = 0; j < cpu_count(); ++j)
		{
			if (i != j && (current_thread[i] == current_thread[j]))
			{
				screen_putstr(kprintf(buf, "%x==%x [%x][%x] idle:%i\n", i, j, current_thread[i], current_thread[j], current_thread[i] == idle_thread[i]));
				for (k = 0; k < cpu_count(); ++k)
					screen_putstr(kprintf(buf, "%i)%x ", k, current_thread[k]));
				screen_putstr(kprintf(buf, "\n"));

				screen_putstr(kprintf(buf, "%x==%x [%x][%x] idle:%i\n", i, j, current_thread[i], current_thread[j], current_thread[i] == idle_thread[i]));
				sched_print();
				while(1) asm("hlt");
			}
		}
	}


	/*
	if (sched_cur_thread() != idle_thread[cpuid()])
	screen_putstr(kprintf(buf, "%x: tick! thread:%x cpu:%x\n", timer_get_ticks(), sched_cur_thread(), cpuid()));
	*/
	/*screen_putstr(kprintf(buf, "lock: %x mem: %x\n", cpuid(), mem_stats()));*/
	
	prev = sched_cur_thread();

	sched_wake_threads();
	sched_put_back();
	
	sched_move_down();
	sched_move_up();
	
	next = sched_pick();

	sched_switch(prev, next);

	return INT_OK;
}

void sched_thread_ready(struct thread *thread)
{
	list_push(&waiting_threads, thread);
}

void sched_init()
{
	struct thread *new_thread;

	section_enter(&lock);
	new_thread = thread_create(proc_get_by_pid(0), (uint32)idle_loop, THREAD_KERNEL);
	section_leave(&lock);

	current_thread[cpuid()] = new_thread;
	idle_thread[cpuid()] = new_thread;
}

void sched_thread_sleep(uint64 ticks)
{
	struct thread *cur = sched_cur_thread();
	cur->wait_time += ticks;
	cur->state = THREAD_WAITING;
	sched_yield();
}

pid_t sched_cur_proc()
{
	return sched_cur_thread()->parent->pid;
}

int sched_thread_select_msg()
{
	/*
	int i = 0;
	char buf[128];
	*/
	struct thread *thread = sched_cur_thread();
	struct process *parent = thread->parent;
	int queue = proc_select_queue(parent);

	while(queue == -1)
	{
		thread->state = THREAD_WAITING;
		/*screen_putstr(kprintf(buf, "%x: looping %x times:%x\n", thread, cpuid(), ++i));*/
		sched_yield();
		queue = proc_select_queue(parent);
	}

	return queue;
}

void sched_yield(void)
{
	sched_tick(0);
	/*asm volatile("int $0x80");*/
}

struct thread *sched_cur_thread(void)
{
	return current_thread[cpuid()];
}

void sched_start_timer()
{
	lapic_set(0x320, 0x00020080);
	lapic_set(0x3E0, 0xB);
	lapic_set(0x380, 0x00100000);
}
