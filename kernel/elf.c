#include "elf.h"

#include "screen.h"
#include "util.h"
#include "memory.h"
#include "process.h"
#include "thread.h"
#include "scheduler.h"

pid_t elf_load(uint8 *buf)
{
	struct elf_header *hdr = (struct elf_header *)buf;
	struct elf_section *sections = (struct elf_section*)(buf + hdr->e_shoff);
	char *str_tab = (char *)(buf + sections[hdr->e_shtrndx].sh_offset);
	int i;
	struct proc_section text, data, bss, rodata;
	struct process *proc;
	struct thread *thread;

	ZEROS(&rodata);
	ZEROS(&text);
	ZEROS(&data);
	ZEROS(&bss);

	for (i = 0; i < hdr->e_shnum; ++i){
		if (!kstrcmp(str_tab + sections[i].sh_name, ".text")){
			text.virt_addr = sections[i].sh_addr;
			text.page_count = page_count(sections[i].sh_size);
			text.phys_addr = page_align((uint32)(buf + sections[i].sh_offset));
		}
		if (!kstrcmp(str_tab + sections[i].sh_name, ".data")){
			data.virt_addr = sections[i].sh_addr;
			data.page_count = page_count(sections[i].sh_size);
			data.phys_addr = page_align((uint32)(buf + sections[i].sh_offset));
		}
		if (!kstrcmp(str_tab + sections[i].sh_name, ".bss")){
			data.virt_addr = sections[i].sh_addr;
			data.page_count = page_count(sections[i].sh_size);
			data.phys_addr = page_align((uint32)(buf + sections[i].sh_offset));
		}
		if (!kstrcmp(str_tab + sections[i].sh_name, ".rodata")){
			rodata.virt_addr = sections[i].sh_addr;
			rodata.page_count = page_count(sections[i].sh_size);
			rodata.phys_addr = page_align((uint32)(buf + sections[i].sh_offset));
		}
	}
	proc = proc_create(text, data, bss, rodata);
	thread = thread_create(proc, hdr->e_entry, THREAD_USERSPACE);
	sched_thread_ready(thread);

	return 0;
}
