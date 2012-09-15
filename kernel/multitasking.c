#include "multitasking.h"
#include "memory.h"
#include "util.h"
#include "gdt.h"
#include "paging.h"
#include "cpu.h"
#include "palloc.h"

#include "process.h"
/*
uint8 proc_map_queue(pid_t pid, queue_id send_to, queue_id receive_from)
{
	uint8 qid = 0;
	struct queue_descr *queues = processes[pid]->msg_queues;

	while(queues[qid].in_use)
		qid++;
	queues[qid].in_use = 1;
	queues[qid].rec_from = receive_from;
	queues[qid].send_to = send_to;
	return qid;
}
*/
uint8 proc_map_queue(pid_t pid, queue_id send_to, queue_id receive_from)
{
	return 0;
}

pid_t proc_cur()
{
	return 0;
}

struct queue_descr proc_get_descr(pid_t pid, queue_id qid)
{
	struct queue_descr res;
	return res;
}

int proc_find_queue(pid_t pid)
{
	return 0;
}

/*
struct queue_descr proc_get_descr(pid_t pid, queue_id qid)
{
	return processes[pid]->msg_queues[qid];
}

int proc_find_queue(pid_t pid)
{
	int i;
	for (i = 0; i < 16; ++i){
		if (processes[pid]->msg_queues[i].in_use && !ipc_empty(processes[pid]->msg_queues[i].rec_from)){
			return i;
		}
	}
	return -1;
}
*/
