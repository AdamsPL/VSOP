#ifndef _PAGING_H
#define _PAGING_H

#include "config.h"
#include "memory.h"

#define PAGE_PRESENT  (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USERMODE (1 << 2)
#define PAGE_WRITETHR (1 << 3)
#define PAGE_NOCACHE  (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY    (1 << 6)
#define PAGE_4MBPAGE  (1 << 7)
#define PAGE_GLOBAL   (1 << 8)

#define PAGE_DIR_VIRT_ADDR 0xFFFFF000

#define KERNEL_PAGE_DIR_PHYS ((uint32)&kpage_dir - 0xC0000000)
extern uint32 kpage_dir;

uint32 page_entry(uint32 addr, uint32 flags);
uint32 page_dir_index(uint32 addr);
uint32 page_table_index(uint32 addr);

uint32 *page_table_virt_addr(int id);

void paging_map(uint32 virt, uint32 phys, uint32 flags);
uint32 paging_get_phys(uint32 addr);
uint8 paging_is_mapped(uint32 virt);

void page_dir_switch(uint32 newpdir);

#endif
