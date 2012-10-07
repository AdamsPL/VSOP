#include "process.h"
#include "config.h"
#include "memory.h"
#include "paging.h"
#include "thread.h"
#include "locks.h"
#include "palloc.h"
#include "screen.h"
#include "scheduler.h"

lock_t lock;

static struct process *processes[MAX_PROCESSES];

static pid_t find_next_pid()
{
	int i = 0;
	section_enter(&lock);
	for (i = 0; i < MAX_PROCESSES; ++i)
		if (!processes[i])
			goto end;
	i = 0;
end:
	section_leave(&lock);
	return i;
}

struct process *proc_create(struct proc_section text, struct proc_section data, struct proc_section bss, struct proc_section rodata)
{
	pid_t pid = find_next_pid();
	uint32 new_dir_phys = page_dir_clone(KERNEL_PAGE_DIR_PHYS);

	processes[pid] = NEW(struct process);

	processes[pid]->pid = pid;

	processes[pid]->text = text;
	processes[pid]->data = data;
	processes[pid]->bss = bss;
	processes[pid]->rodata = rodata;

	processes[pid]->pdir = new_dir_phys;

	page_dir_switch(new_dir_phys);

	paging_map(page_align(text.virt_addr), text.phys_addr, PAGE_PRESENT | PAGE_USERMODE);
	paging_map(page_align(data.virt_addr), data.phys_addr, PAGE_PRESENT | PAGE_USERMODE | PAGE_WRITABLE);
	paging_map(page_align(bss.virt_addr), bss.phys_addr, PAGE_PRESENT | PAGE_USERMODE | PAGE_WRITABLE);
	paging_map(page_align(rodata.virt_addr), rodata.phys_addr, PAGE_PRESENT | PAGE_USERMODE);

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
	/*//TODO: Race conditions!!!*/
	kstrncpy((uint8*)proc->name, (uint8*)name, PROC_MAX_NAME_LEN);
	return 0;
}

struct process *proc_create_kernel_proc()
{
   	pid_t pid = find_next_pid();
	struct process *new = NEW(struct process);

	processes[pid] = new;
	new->pid = pid;
	new->pdir = KERNEL_PAGE_DIR_PHYS;

	return new;
}

int proc_attach_queue(struct process *proc, struct msg_queue *send_queue, struct msg_queue *recv_queue)
{
	int i;

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

	proc->selecting_thread = sched_cur_thread();

	for (i = 0; i < PROC_MAX_QUEUES; ++i)
	{
		if (!msg_queues[i].recv)
			continue;
		if (!queue_is_empty(msg_queues[i].recv))
			return i;
	}

	return -1;
}

struct queue_descr *proc_get_descr(struct process *proc, int id)
{
	return proc->msg_queues + id;
}
