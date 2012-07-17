#ifndef _MULTITASKING_H
#define _MULTITASKING_H

#include "config.h"
#include "interrupts.h"
#include "ipc.h"

#define NULL_THREAD 0
#define THREAD_EXISTS 1
#define THREAD_IS_RDY 2

#define NULL_PROCESS 0
#define PROC_EXISTS 1

#define PROC_MAX_QUEUES 0x100

struct proc_section
{
	uint32 virt_addr;
	uint32 phys_addr;
	uint32 page_count;
};

struct process
{
	struct proc_section text;
	struct proc_section data;
	struct proc_section bss;
	uint32 pdir;
	struct queue_descr msg_queues[PROC_MAX_QUEUES];
};

struct thread
{
	uint32 starting_priority;
	uint32 current_priority;
	proc_id parent;
	uint32 kernel_stack;
	thread_id next;
	uint32 ebp;
	uint32 esp;
	uint32 eip;
};

struct shed_queue
{
	thread_id head;
	thread_id tail;
};

struct shed_data
{
	struct shed_queue priorities[MAX_PRIORITIES];
	uint32 thread_count;
	thread_id cur_thread;
};

extern struct process *processes[MAX_PROCESSES];
extern struct thread *threads[MAX_THREADS];

thread_id thread_create(proc_id parent, uint32 entry);
proc_id proc_create(struct proc_section text, struct proc_section data, struct proc_section bss, uint32 entry);

uint8 proc_map_queue(proc_id pid, queue_id send_to, queue_id receive_from);

void sheduler_enqueue(thread_id thread);
void sheduler_tick(struct thread_state *cur_state);

proc_id proc_cur();
struct queue_descr proc_get_descr(proc_id pid, queue_id qid);
int proc_find_queue(proc_id pid);

void task_switch(struct thread_state *cur_state, thread_id from_thread, thread_id to_thread);
#endif 
