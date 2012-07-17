#include "mboot.h"
#include "memory.h"
#include "elf.h"
#include "palloc.h"
#include "paging.h"

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

	mem_phys_init(1024 * mboot->mem_upper + 0x100000);

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
