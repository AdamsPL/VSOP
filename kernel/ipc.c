#include "ipc.h"

#include "memory.h"
#include "process.h"
#include "thread.h"
#include "scheduler.h"
#include "screen.h"

struct msg_queue *queue_create()
{
	struct msg_queue *result = NEW(struct msg_queue);
	return result;
}

int ipc_connect(pid_t p1, pid_t p2)
{
	struct msg_queue *q1 = queue_create(p1);
	struct msg_queue *q2 = queue_create(p2);

	proc_attach_queue(proc_get_by_pid(p2), q2, q1);
	return proc_attach_queue(proc_get_by_pid(p1), q1, q2);
}

static inline int next(int value)
{
	return (value+1) % QUEUE_LEN;
}

int ipc_send(struct queue_descr *descr, uint8 *ptr, uint16 size)
{
	int count = 0;
	struct msg_queue *queue = descr->send;
	struct process *parent = proc_get_by_pid(queue->header.owner);
	struct thread *thread = parent->selecting_thread;

	while(next(queue->header.write) != queue->header.read){
		queue->buf[queue->header.write] = *ptr++;
		count++;
		queue->header.write = next(queue->header.write);
		if (count == size)
			break;
	}

	if (thread)
		sched_thread_ready(thread);

	return count;
}

int ipc_receive(struct queue_descr *descr, uint8 *ptr, uint16 size)
{
	int count = 0;
	struct msg_queue *queue = descr->recv;

	while(queue->header.write != queue->header.read){
		*ptr++ = queue->buf[queue->header.read];
		count++;
		queue->header.read = next(queue->header.read);
		if (count == size)
			break;
	}
	return count;
}

int queue_is_empty(struct msg_queue *this)
{
	return (this->header.read == this->header.write);
}
