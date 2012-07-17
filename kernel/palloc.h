#ifndef _PALLOC_H
#define _PALLOC_H

#include "config.h"

void mem_phys_init();
void mem_phys_reserve(uint32 addr);
void mem_phys_reserve_range(uint32 addr, uint32 pages);
void mem_phys_free(uint32 addr);
void mem_phys_free_range(uint32 addr, uint32 pages);
uint32 mem_phys_alloc();
void mem_phys_dump();

#endif
