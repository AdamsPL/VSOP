#include "ipc.h"

#include "multitasking.h"
#include "memory.h"

static struct msg_queue *queues[MAX_QUEUES];

queue_id queue_create()
{
	queue_id id;
	for (id = 0; id < MAX_QUEUES; ++id){
		if (queues[id] == 0){
			queues[id] = kmalloc(sizeof(struct msg_queue));
			return id;
		}
	}
	return 0;
}

uint8 ipc_connect(pid_t p1, pid_t p2)
{
	queue_id q1, q2;
	q1 = queue_create();
	q2 = queue_create();

	proc_map_queue(p2, q2, q1);
	return proc_map_queue(p1, q1, q2);
}

static inline int next(int value)
{
	return (value+1) % QUEUE_LEN;
}

int ipc_send(queue_id id, uint8 *ptr, uint16 size)
{
	int status = 0;
	struct msg_queue *queue = queues[id];

	while(next(queue->header.write) != queue->header.read){
		queue->buf[queue->header.write] = *ptr++;
		status++;
		queue->header.write = next(queue->header.write);
		if (status == size)
			break;
	}
	return status;
}

int ipc_receive(queue_id id, uint8 *ptr, uint16 size)
{
	int status = 0;
	struct msg_queue *queue = queues[id];

	while(queue->header.write != queue->header.read){
		*ptr++ = queue->buf[queue->header.read];
		status++;
		queue->header.read = next(queue->header.read);
		if (status == size)
			break;
	}
	return status;
}

int ipc_empty(queue_id id)
{
	struct msg_queue *queue = queues[id];
	return (queue->header.write == queue->header.read);
}
