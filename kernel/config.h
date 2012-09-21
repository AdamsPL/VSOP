#ifndef _CONFIG_H
#define _CONFIG_H

#define MAX_THREADS (1 << 16)
#define MAX_PROCESSES (1 << 16)
#define MAX_QUEUES (1 << 16)
#define MAX_CPU 8
#define DEFAULT_PRIORITY 1

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGES (1 << (32-PAGE_SHIFT))

#define PROC_MAX_STREAMS 0x100

#define ADDR_BASE 0xC0000000
#define VIRT_TO_PHYS(x) ((uint32)x - ADDR_BASE)

#define STACK_OFFSET (PAGE_SIZE - 0x10)

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned long long uint64;
typedef signed long long int64;

typedef uint16 pid_t;
typedef uint16 queue_id;
typedef uint32 size_t;
typedef uint32 *lock_t;

#endif
