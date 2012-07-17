#include "multitasking.h"
#include "memory.h"
#include "util.h"
#include "gdt.h"
#include "paging.h"
#include "cpu.h"
#include "palloc.h"

#include "interrupts.h"

struct shed_data sheduler_info[MAX_CPU];

struct process *processes[MAX_PROCESSES];
struct thread *threads[MAX_THREADS];

#include "screen.h"
#include "util.h"

void task_switch(struct thread_state *cur_state, thread_id from_thread, thread_id to_thread)
{
	uint32 cpu = cpuid();
	uint32 esp, ebp, eip;

	if (from_thread == to_thread)
		return;

	asm volatile("movl %%esp, %0" : "=r"(esp));
	asm volatile("movl %%ebp, %0" : "=r"(ebp));

	threads[from_thread]->esp = esp;
	threads[from_thread]->ebp = ebp;
	threads[from_thread]->eip = eip;

	tss_set_stack(cpu, threads[to_thread]->kernel_stack);

	sheduler_info[cpu].cur_thread = to_thread;

	if (threads[from_thread]->parent != threads[to_thread]->parent){
		page_dir_switch(processes[threads[to_thread]->parent]->pdir);
	}

	esp = threads[to_thread]->esp;
	eip = threads[to_thread]->eip;
	ebp = threads[to_thread]->ebp;

}

thread_id find_next_tid()
{
	uint32 i = 0;
	for (i = 1; i < MAX_THREADS; ++i)
	{
		if (threads[i])
			continue;
		return i;
	}
	return NULL_PROCESS;
}

proc_id find_next_pid()
{
	uint32 i;
	for (i = 1; i < MAX_PROCESSES; ++i)
	{
		if (processes[i])
			continue;
		return i;
	}
	return NULL_PROCESS;
}

proc_id proc_create(struct proc_section text, struct proc_section data, struct proc_section bss, uint32 entry)
{
	proc_id pid = find_next_pid();
	uint32 *new_dir;
	uint32 new_dir_phys;

	processes[pid] = kmalloc(sizeof(struct process));

	processes[pid]->text = text;
	processes[pid]->data = data;
	processes[pid]->bss = bss;
	
	new_dir = (uint32*)0x900000;
	new_dir_phys = mem_phys_alloc(1);
	processes[pid]->pdir = new_dir_phys;

	paging_map((uint32)new_dir, new_dir_phys, PAGE_USERMODE | PAGE_PRESENT);
	kmemcpy((uint8*)new_dir, (uint8*)KERNEL_PAGE_DIR_PHYS, PAGE_SIZE);
	new_dir[1023] = page_entry(new_dir_phys, PAGE_WRITABLE | PAGE_PRESENT);

	page_dir_switch(new_dir_phys);
	paging_map(page_align(text.virt_addr), text.phys_addr, PAGE_PRESENT | PAGE_USERMODE);
	paging_map(page_align(data.virt_addr), data.phys_addr, PAGE_PRESENT | PAGE_USERMODE | PAGE_WRITABLE);

	sheduler_enqueue(thread_create(pid, entry));

	page_dir_switch(KERNEL_PAGE_DIR_PHYS);
	return pid;
}

thread_id thread_create(proc_id parent, uint32 entry)
{
	thread_id tid = find_next_tid();
	uint32 stack = 0xa00000;

	threads[tid] = kmalloc(sizeof(struct thread));
	paging_map(stack, mem_phys_alloc(), PAGE_USERMODE | PAGE_PRESENT | PAGE_WRITABLE);

	threads[tid]->parent = parent;
	threads[tid]->starting_priority = DEFAULT_PRIORITY;
	threads[tid]->kernel_stack = (uint32)kmalloc(PAGE_SIZE) + PAGE_SIZE;

	stack += PAGE_SIZE;

	threads[tid]->esp = stack;
	threads[tid]->ebp = stack;
	threads[tid]->eip = entry;
	return tid;
}

void shed_queue_enqueue(struct shed_queue *queue, thread_id thread)
{
	thread_id last = queue->tail;
	threads[thread]->next = NULL_THREAD;

	if (thread == NULL_THREAD)
		return;

	if (last == NULL_THREAD){
		queue->tail = thread;
		queue->head = thread;
	}else{
		threads[last]->next = thread;
		queue->tail = thread;
	}
}

thread_id shed_queue_dequeue(struct shed_queue *queue)
{
	thread_id result = queue->head;
	if (result == NULL_THREAD)
		return result;
	queue->head = threads[result]->next;
	if (queue->tail == result)
		queue->tail = queue->head;
	threads[result]->next = NULL_THREAD;
	return result;
}

void sheduler_enqueue(thread_id thread)
{
	uint8 cpu = cpuid();

	shed_queue_enqueue(&sheduler_info[cpu].priorities[threads[thread]->starting_priority], thread);
	sheduler_info[cpu].thread_count++;
}

void sheduler_tick(struct thread_state *state)
{
	uint8 cpu = cpuid();
	thread_id cur_thread = sheduler_info[cpu].cur_thread;
	uint32 i;
	thread_id next_thread;

	threads[cur_thread]->current_priority++;
	if (threads[cur_thread]->current_priority >= MAX_PRIORITIES)
		shed_queue_enqueue(&sheduler_info[cpu].priorities[MAX_PRIORITIES-1], cur_thread);
	else
		shed_queue_enqueue(&sheduler_info[cpu].priorities[threads[cur_thread]->current_priority], cur_thread);
	
	for (i = 0; i < MAX_PRIORITIES; ++i){
		next_thread = shed_queue_dequeue(&sheduler_info[cpu].priorities[i]);
		if (next_thread != NULL_THREAD){
			sheduler_info[cpu].cur_thread = NULL_THREAD;
			task_switch(state, cur_thread, next_thread);
			return;
		}
	}
}

uint8 proc_map_queue(proc_id pid, queue_id send_to, queue_id receive_from)
{
	uint8 qid = 0;
	struct queue_descr *queues = processes[pid]->msg_queues;

	while(queues[qid].in_use)
		qid++;
	queues[qid].in_use = 1;
	queues[qid].rec_from = receive_from;
	queues[qid].send_to = send_to;
	return qid;
}

proc_id proc_cur()
{
	uint8 cpu = cpuid();
	thread_id tid = sheduler_info[cpu].cur_thread;
	
	if (!tid)
		return 0;
	return threads[tid]->parent;
}

struct queue_descr proc_get_descr(proc_id pid, queue_id qid)
{
	return processes[pid]->msg_queues[qid];
}

int proc_find_queue(proc_id pid)
{
	int i;
	for (i = 0; i < 16; ++i){
		if (processes[pid]->msg_queues[i].in_use && !ipc_empty(processes[pid]->msg_queues[i].rec_from)){
			return i;
		}
	}
	return -1;
}
