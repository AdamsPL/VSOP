#ifndef _PROCESS_H
#define _PROCESS_H

#include "config.h"

#include "ipc.h"

#define PROC_MAX_QUEUES 0x100
#define PROC_MAX_NAME_LEN 32

struct proc_section
{
	uint32 virt_addr;
	uint32 phys_addr;
	uint32 page_count;
};

struct thread;

struct process
{
	pid_t pid;
	struct proc_section text;
	struct proc_section data;
	struct proc_section bss;
	uint32 pdir;
	struct thread *threads;
	char name[PROC_MAX_NAME_LEN];
	struct queue_descr msg_queues[PROC_MAX_QUEUES];
};

struct process *proc_create(struct proc_section text, struct proc_section data, struct proc_section bss);
struct process *proc_create_kernel_proc();
struct process *proc_get_by_pid(pid_t pid);
struct process *proc_get_by_name(char *name);
uint8 proc_register(struct process *proc, char *name);
#endif
