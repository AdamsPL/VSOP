#include "memory.h"
#include "paging.h"
#include "kmalloc.h"
#include "interrupts.h"
#include "palloc.h"

#include "screen.h"

#include "locks.h"

extern uint32 end;
extern uint32 code;

uint32 heap;
static lock_t lock = 0;
uint32 stats = 0;

inline uint32 page_count(uint32 bytes)
{
	return (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
}

inline uint32 page_align(uint32 addr)
{
	return ((addr >> PAGE_SHIFT) << PAGE_SHIFT);
}

void *kbrk(uint32 pages)
{
	int i;
	uint32 res = heap;

	for (i = 0; i < pages; ++i){
		if (!paging_is_mapped(heap))
			paging_map(heap, mem_phys_alloc(), PAGE_PRESENT | PAGE_WRITABLE);
		heap += PAGE_SIZE;
	}
	return (void*)res;
}


void mem_init(struct mmap *mmap, uint32 length)
{
	uint8 *end = (uint8*)mmap + length;
	uint8 *ptr = (uint8*)mmap;
	heap = 0xC3000000;

	mem_allocator_init(&allocator);

	while (ptr < end) {
		mmap = (struct mmap*)ptr;
		if (mmap->type == 1){
			uint32 addr = page_align(mmap->base_low + PAGE_SIZE - 1);
			uint32 end = page_align(mmap->base_low + mmap->length_low);
			uint32 pages = (end - addr) / PAGE_SIZE;
			mem_phys_free_range(addr, pages);
		}
		ptr += mmap->size + 4;
	}
	
	mem_phys_reserve(0x00000000);
	mem_phys_reserve_range(0x00100000, 0x100000 / PAGE_SIZE);
	mem_phys_reserve(LAPIC_BASE);
	mem_phys_reserve(IOAPIC_BASE);

	paging_map(0xFEC00000, 0xFEC00000, PAGE_PRESENT | PAGE_WRITABLE);
	paging_map(0xFEE00000, 0xFEE00000, PAGE_PRESENT | PAGE_WRITABLE); 

}

void *kmalloc(uint32 size)
{
	uint8 *buf;

	section_enter(&lock);
	stats += size;
	buf	= (uint8*)mem_alloc(&allocator, size);
	kmemset(buf, 0x00, size);

	section_leave(&lock);

	return buf;
}

void kfree(void *ptr, uint32 size)
{
	section_enter(&lock);
	stats -= size;
	mem_free(&allocator, ptr, size);
	section_leave(&lock);
}

uint32 mem_stats()
{
	return stats;
}
