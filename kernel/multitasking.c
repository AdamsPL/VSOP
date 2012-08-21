#include "multitasking.h"
#include "memory.h"
#include "util.h"
#include "gdt.h"
#include "paging.h"
#include "cpu.h"
#include "palloc.h"

#include "process.h"
/*
pid_t proc_create(struct proc_section text, struct proc_section data, struct proc_section bss, uint32 entry)
{
	pid_t pid = find_next_pid();
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
*/
/*
thread_id thread_create(pid_t parent, uint32 entry)
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
*/
/*
uint8 proc_map_queue(pid_t pid, queue_id send_to, queue_id receive_from)
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
*/
uint8 proc_map_queue(pid_t pid, queue_id send_to, queue_id receive_from)
{
	return 0;
}

pid_t proc_cur()
{
	return 0;
}

struct queue_descr proc_get_descr(pid_t pid, queue_id qid)
{
	struct queue_descr res;
	return res;
}

int proc_find_queue(pid_t pid)
{
	return 0;
}

/*
pid_t proc_cur()
{
	uint8 cpu = cpuid();
	thread_id tid = sheduler_info[cpu].cur_thread;
	
	if (!tid)
		return 0;
	return threads[tid]->parent;
}

struct queue_descr proc_get_descr(pid_t pid, queue_id qid)
{
	return processes[pid]->msg_queues[qid];
}

int proc_find_queue(pid_t pid)
{
	int i;
	for (i = 0; i < 16; ++i){
		if (processes[pid]->msg_queues[i].in_use && !ipc_empty(processes[pid]->msg_queues[i].rec_from)){
			return i;
		}
	}
	return -1;
}
*/
