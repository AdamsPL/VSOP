#include "process.h"
#include "config.h"
#include "memory.h"
#include "paging.h"
#include "thread.h"

static struct process *processes[MAX_PROCESSES];

static pid_t find_next_pid()
{
	int i = 1;
	for (i = 1; i < MAX_PROCESSES; ++i)
		if (processes[i])
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

