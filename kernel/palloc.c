#include "palloc.h"
#include "memory.h"

#define BITS 32
#define ENTRIES (PAGES / BITS)

#include "screen.h"
#include "util.h"
#include "locks.h"

static lock_t lock = 0;

static uint32 memory_map[ENTRIES + 1];

void mem_phys_init()
{
	/*array guard*/
	memory_map[ENTRIES] = 0xFFFFFFFF;
}

void mem_phys_free(uint32 addr)
{
	uint32 page = (addr / PAGE_SIZE);
	uint32 id = page / BITS;
	uint32 bit = page % BITS;

	memory_map[id] |= (1 << bit);
}

void mem_phys_reserve(uint32 addr)
{
	uint32 page = (addr / PAGE_SIZE);
	uint32 id = page / BITS;
	uint32 bit = page % BITS;

	memory_map[id] &= ~(1 << bit);
}

uint32 mem_phys_alloc()
{
	uint32 i = 0;
	uint8 bit = 0;
	uint32 addr;
	uint32 tmp;

	section_enter(&lock);

	while (!memory_map[i]){
		++i;
	}
	if (i == ENTRIES){
		/*PANIC!*/
		return 0xDEADC0DE;
	}

	tmp = memory_map[i];

	while (tmp % 2 == 0){
		tmp >>= 1;
		++bit;
	}
	addr = PAGE_SIZE * (BITS * i + bit);
	mem_phys_reserve(addr);

	tmp = PAGE_SIZE * (BITS * i + bit);

	section_leave(&lock);
	return tmp;
}

void mem_phys_dump()
{
	char buf[128];
	uint32 i;
	for (i = 0; i < ENTRIES; ++i){
		if (memory_map[i])
			screen_putstr(kprintf(buf, "%i %x\n", i, memory_map[i]));
	}
}

void mem_phys_free_range(uint32 addr, uint32 pages)
{
	uint32 i;
	for (i = 0; i < pages; ++i){
		mem_phys_free(addr + PAGE_SIZE * i);
	}
}

void mem_phys_reserve_range(uint32 addr, uint32 pages)
{
	uint32 i;
	for (i = 0; i < pages; ++i){
		mem_phys_reserve(addr + PAGE_SIZE * i);
	}
}
