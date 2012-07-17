#ifndef _KMALLOC_H
#define _KMALLOC_H

struct mem_allocator;

extern struct mem_allocator allocator;

void mem_allocator_init(struct mem_allocator *aloc);
void *mem_alloc(struct mem_allocator *alloc, uint32 size);
void mem_free(struct mem_allocator *alloc, void *addr, uint32 size);

#endif


