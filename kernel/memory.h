#ifndef _MEMORY_H
#define _MEMORY_H

#include "config.h"
#include "util.h"
#include "mboot.h"

void mem_init(struct mmap *map, uint32 length);
void *kbrk(uint32 pages);

uint32 page_count(uint32 bytes);
inline uint32 page_align(uint32 addr);
inline uint32 kernel_size();

void *kmalloc(uint32 size);
#endif
