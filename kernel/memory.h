#ifndef _MEMORY_H
#define _MEMORY_H

#include "config.h"
#include "util.h"
#include "mboot.h"

void mem_init(struct mmap *map, uint32 length);
void *kbrk(uint32 pages);

uint32 page_count(uint32 bytes);
inline uint32 page_align(uint32 addr);
uint32 mem_stats();

void *kmalloc(uint32 size);
void kfree(void *ptr, uint32 size);

#define NEW(type) (type *) kmemset(kmalloc(sizeof(type)), 0, sizeof(type));
#define ZEROS(ptr) kmemset((uint8 *)(ptr), 0, sizeof(*ptr));
#define DELETE(ptr) kfree((void*)(ptr), sizeof(*ptr));

#endif
