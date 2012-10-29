#include "mboot.h"
#include "memory.h"
#include "elf.h"
#include "palloc.h"
#include "paging.h"
#include "screen.h"
#include "cpu.h"

struct module_info{
	uint32 mod_start;
	uint32 mod_end;
	uint32 string;
	uint32 reserved;
};

struct mboot {
	uint32 flags;
	uint32 mem_lower;
	uint32 mem_upper;
	uint32 boot_device;
	uint32 cmdline;
	uint32 mods_count;
	uint32 mods_addr;
	uint32 syms[4];
	uint32 mmap_length;
	struct mmap *mmap_addr;
};

void mboot_parse(struct mboot *mboot)
{
	struct module_info *mod_info;
	uint32 i;

	mem_phys_init();

	mem_init(mboot->mmap_addr, mboot->mmap_length);

	mem_phys_reserve(page_align((uint32)mboot));
	mod_info = (struct module_info*) mboot->mods_addr;
	for (i = 0; i < mboot->mods_count; ++i){
		uint32 pages = page_count(mod_info->mod_end - page_align(mod_info->mod_start));
		mem_phys_reserve_range(page_align(mod_info->mod_start), pages);
		mem_phys_reserve(page_align((uint32)mod_info));
		mod_info++;
	}
}

void mboot_load_modules(struct mboot *mboot)
{
	struct module_info *mod_info;
	uint32 i;

	mod_info = (struct module_info*) mboot->mods_addr;
	for (i = 0; i < mboot->mods_count; ++i){
		elf_load((uint8*)mod_info->mod_start);
		mod_info++;
	}
}

static char *cmdline_find(char *cmdline, char *txt)
{
	while(*cmdline)
	{
		if (ksubstr(cmdline, txt))
			return cmdline;
		cmdline++;
	}
	return 0;
}

void mboot_parse_cmdline(struct mboot *mboot)
{
	char *cmdline = (char*)0x70000; 
	char *max_cpu_param = 0;
	char buf[64];

	if (mboot->flags && 0x04)
	{
		paging_map((uint32)cmdline, mboot->cmdline, PAGE_PRESENT);
		screen_putstr(cmdline);
		screen_putstr("\n");
		max_cpu_param = cmdline_find(cmdline, "max_cpu=");
		if (!max_cpu_param)
			return;
		max_cpu_param += 8;
		maximum_cpu = *max_cpu_param - '0';
		screen_putstr(kprintf(buf, "found %x...\n", maximum_cpu));
	}
}
