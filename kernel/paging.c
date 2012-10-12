#include "paging.h"
#include "memory.h"
#include "palloc.h"
#include "screen.h"

inline uint32 page_entry(uint32 addr, uint32 flags)
{
	flags &= 0xFFF;
	addr &= 0xFFFFF000;
	return (addr | flags);
}

inline uint32 page_dir_index(uint32 addr)
{
	return addr >> 22;
}

inline uint32 page_table_index(uint32 addr)
{
	return (addr >> 12) & 0x3FF;
}

inline uint32 *page_table_virt_addr(int id)
{
	return (uint32*)(PAGE_DIR_VIRT_ADDR - 0x400000 + (id+1) * 0x1000);
}

uint8 paging_is_mapped(uint32 virt)
{
	uint32 *page_dir = (uint32*)PAGE_DIR_VIRT_ADDR;
	uint32 index = page_dir_index(virt);
	uint32 *page_table;

	if (page_dir[index] & PAGE_PRESENT){
		page_table = page_table_virt_addr(index);
	}else
		return 0;

	return (page_table[page_table_index(virt)] & PAGE_PRESENT);
}

void paging_map(uint32 virt, uint32 phys, uint32 flags)
{
	uint32 *page_dir = (uint32*)PAGE_DIR_VIRT_ADDR;
	uint32 index = page_dir_index(virt);
	uint32 *page_table;

	if (page_dir[index] & PAGE_PRESENT){
		page_table = page_table_virt_addr(index);
	}else{
		page_dir[index] = page_entry(mem_phys_alloc(), PAGE_WRITABLE | PAGE_USERMODE | PAGE_PRESENT);
		page_table = page_table_virt_addr(index);
		kmemset32(page_table, 0x00000000, 1024);
	}
	page_table[page_table_index(virt)] = page_entry(phys, flags);
	asm volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

void page_dir_switch(uint32 newpdir)
{
	asm volatile("mov %0, %%cr3":: "r"(newpdir));
}

uint32 paging_get_phys(uint32 addr)
{
	uint32 index = page_dir_index(addr);
	uint32 *ptr = page_table_virt_addr(index);
	if (!paging_is_mapped(addr))
		return 0x00;
	index = page_table_index(addr);
	return (ptr[index] & 0xFFFFF000);
}

uint32 page_dir_clone(uint32 olddir)
{
	static uint32 *olddir_virt = (uint32*)0xcccc0000;
	static uint32 *newdir_virt = (uint32*)0xdddd0000;
	uint32 newdir;
	int i;

	newdir = mem_phys_alloc();

	paging_map((uint32)olddir_virt, olddir & 0xFFFFF000, PAGE_WRITABLE | PAGE_PRESENT);
	paging_map((uint32)newdir_virt, newdir & 0xFFFFF000, PAGE_WRITABLE | PAGE_PRESENT);

	for (i = 0; i < 1023; ++i)
		newdir_virt[i] = olddir_virt[i];
	newdir_virt[1023] = page_entry(newdir, PAGE_PRESENT);

	paging_map((uint32)olddir_virt, 0, 0);
	paging_map((uint32)newdir_virt, 0, 0);

	return newdir;
}
