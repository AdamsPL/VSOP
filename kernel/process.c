#include "process.h"
#include "config.h"
#include "memory.h"
#include "paging.h"
#include "thread.h"

static struct process *processes[MAX_PROCESSES];

static pid_t find_next_pid()
{
	int i = 0;
	for (i = 0; i < MAX_PROCESSES; ++i)
		if (!processes[i])
			return i;
	return 0;
}

struct process *proc_create(struct proc_section text, struct proc_section data, struct proc_section bss)
{
	pid_t pid = find_next_pid();
	uint32 *new_dir;
	uint32 new_dir_phys;
	struct thread *thread;

	processes[pid] = NEW(struct process);

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

	page_dir_switch(KERNEL_PAGE_DIR_PHYS);

	return processes[pid];
}

struct process *proc_get_by_pid(pid_t pid)
{
	return processes[pid];
}

struct process *proc_get_by_name(char *name)
{
	int i = 1;
	for (i = 1; i < MAX_PROCESSES; ++i)
		if (processes[i] && !kstrcmp(name, processes[i]->name))
			return processes[i];
	return 0;
}

int proc_register(struct process *proc, char *name)
{
	//TODO: Race conditions!!!
	kstrncpy(proc->name, name, PROC_MAX_NAME_LEN);
	return 0;
}

struct process *proc_create_kernel_proc()
{
	struct process *new = NEW(struct process);
   	pid_t pid = find_next_pid();

	processes[pid] = new;
	new->pid = pid;
	new->pdir = KERNEL_PAGE_DIR_PHYS;

	return new;
}

int proc_attach_queue(struct process *proc, struct msg_queue *send_queue, struct msg_queue *recv_queue)
{
	int i;
	//TODO: Add locking
	for (i = 0; i < PROC_MAX_QUEUES; ++i)
	{
		if (!proc->msg_queues[i].send && !proc->msg_queues[i].recv)
			break;
	}
	if (i == PROC_MAX_QUEUES)
		return -1;

	proc->msg_queues[i].send = send_queue;
	proc->msg_queues[i].recv = recv_queue;

	proc->msg_queues[i].recv->header.owner = proc->pid;
	proc->msg_queues[i].recv->header.descr = i;

	return i;
}

int proc_select_queue(struct process *proc)
{
	int i;
	struct queue_descr *msg_queues = proc->msg_queues;

	for (i = 0; i < PROC_MAX_QUEUES; ++i)
	{
		if (!msg_queues[i].recv)
			continue;
		if (!queue_is_empty(msg_queues[i].recv))
			return i;
	}
	return -1;
}
