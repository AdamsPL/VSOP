#ifndef _CONFIG_H
#define _CONFIG_H

#define MAX_THREADS (1 << 16)
#define MAX_PROCESSES (1 << 16)
#define MAX_CPU 8
#define DEFAULT_PRIORITY 1

#define PROC_MAX_QUEUES 0x10

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGES (1 << (32-PAGE_SHIFT))

#define ADDR_BASE 0xC0000000
#define VIRT_TO_PHYS(x) ((uint32)x - ADDR_BASE)

#define STACK_OFFSET (PAGE_SIZE - 0x10)

#define UNUSED(x) (void)(x);
#define MAX_PRIORITY 5

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned long uint64;

typedef uint32 pid_t;
typedef uint16 queue_id;
typedef uint32 size_t;
typedef volatile int lock_t;

#endif
