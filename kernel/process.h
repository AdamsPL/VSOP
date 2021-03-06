#ifndef _PROCESS_H
#define _PROCESS_H

#include "config.h"
#include "list.h"
#include "stream.h"

#define PROC_MAX_NAME_LEN 64

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
	struct proc_section rodata;
	uint32 pdir;
	char name[PROC_MAX_NAME_LEN];

	struct iostream_descriptors iodescr;
};

struct process *proc_create(struct proc_section text, struct proc_section data, struct proc_section bss, struct proc_section rodata);
struct process *proc_create_kernel_proc();
struct process *proc_get_by_pid(pid_t pid);
struct process *proc_get_by_name(char *name);
int proc_register(struct process *proc, char *name);

#endif
