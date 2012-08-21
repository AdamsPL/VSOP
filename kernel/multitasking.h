#ifndef _MULTITASKING_H
#define _MULTITASKING_H

#include "config.h"
#include "interrupts.h"
#include "ipc.h"

#define NULL_THREAD 0
#define THREAD_EXISTS 1
#define THREAD_IS_RDY 2

#define NULL_PROCESS 0
#define PROC_EXISTS 1

#define PROC_MAX_QUEUES 0x100

uint8 proc_map_queue(pid_t pid, queue_id send_to, queue_id receive_from);
struct queue_descr proc_get_descr(pid_t pid, queue_id qid);
int proc_find_queue(pid_t pid);

#endif 
