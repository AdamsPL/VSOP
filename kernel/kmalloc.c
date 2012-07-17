#include "config.h"
#include "memory.h"

#define MEMALOC_SIZE 0x4000
#define ALOCATORS 16
#define NODES ((MEMALOC_SIZE - sizeof(uint16) * ALOCATORS) / sizeof(struct mem_node))
#define COUNT(x) (sizeof(x) / sizeof(*x))

struct mem_node
{
	uint32 addr;
	uint16 next;
};

struct mem_allocator
{
	uint16 heads[ALOCATORS];
	struct mem_node nodes[NODES];
};

struct mem_allocator allocator;

static inline uint32 list_size(uint8 list)
{
	return (1 << (list + 3));
}

static void list_clean(struct mem_allocator *alloc, uint8 list);

void mem_allocator_init(struct mem_allocator *aloc)
{
	int i;
	for (i = 0; i < COUNT(aloc->nodes); ++i){
		aloc->nodes[i].addr = 0;
		aloc->nodes[i].next = i+1;
	}
	aloc->nodes[COUNT(aloc->nodes) - 1].next = 0;
	aloc->heads[0] = 1;

	for (i = 1; i < COUNT(aloc->heads); ++i){
		aloc->heads[i] = 0;
	}
}

static uint16 node_new(struct mem_allocator *alloc, uint32 addr)
{
	uint16 res = alloc->heads[0];
	alloc->heads[0] = alloc->nodes[res].next;
	alloc->nodes[res].addr = addr;
	alloc->nodes[res].next = 0;
	return res;
}

static void node_free(struct mem_allocator *alloc, uint16 node)
{
	alloc->nodes[node].next = alloc->heads[0];
	alloc->nodes[node].addr = 0;
	alloc->heads[0] = node;
}


static void list_put(struct mem_allocator *alloc, uint8 list, uint16 new_node)
{
	uint16 cur = alloc->heads[list];
	struct mem_node *nodes = alloc->nodes;
	uint16 prev = 0;

	while(cur && (nodes[new_node].addr > nodes[cur].addr)){
		prev = cur;
		cur = nodes[cur].next;
	}

	if (prev){
		nodes[new_node].next = cur;
		nodes[prev].next = new_node;
	}else{
		nodes[new_node].next = cur;
		alloc->heads[list] = new_node;
	}

	if (list < ALOCATORS - 1)
		list_clean(alloc, list);
}

static uint32 list_get(struct mem_allocator *alloc, uint16 list)
{
	uint16 elem;
	uint32 addr;

	if (list >= ALOCATORS)
		return 0;

	elem = alloc->heads[list];
	if (elem){
		alloc->heads[list] = alloc->nodes[elem].next;
		addr = alloc->nodes[elem].addr;
		node_free(alloc, elem);
		return addr;
	}
	
	addr = list_get(alloc, list+1);
	if (addr){
		elem = node_new(alloc, addr + list_size(list + 1) / 2);
		list_put(alloc, list, elem);
	}
	return addr;
}

void *mem_alloc(struct mem_allocator *alloc, uint32 size)
{
	uint32 i;
	uint32 block_size = list_size(1);
	void *result;
	uint16 new_node;

	for (i = 1; i < COUNT(alloc->nodes); ++i){
		if (size <= block_size)
			break;
		block_size <<= 1;
	}

	if (i == COUNT(alloc->nodes)){
		result = (void*)0;
	}else{
		result = (void*)list_get(alloc, i);
	}

	if (result)
		return result;

	new_node = node_new(alloc, (uint32)kbrk(1));
	list_put(alloc, 9, new_node);
	return mem_alloc(alloc, size);
}

void mem_free(struct mem_allocator *alloc, void *addr, uint32 size)
{
	uint8 i = 1;
	uint32 block_size = list_size(1);
	uint16 new_node = node_new(alloc, (uint32)addr);

	while(size > block_size){
		i++;
		block_size <<= 1;
	}
	list_put(alloc, i, new_node);
}

static void list_clean(struct mem_allocator *alloc, uint8 list)
{
	struct mem_node *nodes = alloc->nodes;
	uint16 prev = 0;
	uint16 cur = alloc->heads[list];
	uint16 next;
	uint32 size = list_size(list);
	uint32 align = list_size(list+1);

	while(cur){
		next = nodes[cur].next;
		if (next && nodes[cur].addr % align == 0 && nodes[cur].addr + size == nodes[next].addr){
			if (prev)
				nodes[prev].next = nodes[next].next;
			else
				alloc->heads[list] = nodes[next].next;
			list_put(alloc, list + 1, cur);
			cur = nodes[next].next;
			node_free(alloc, next);
		}else{
			prev = cur;
			cur = next;
		}
	}

}
