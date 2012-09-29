#ifndef _IPC_H
#define _IPC_H

#include "config.h"

struct msg_queue_header
{
	pid_t owner;
	uint16 read;
	uint16 write;
};

#define QUEUE_LEN (PAGE_SIZE - sizeof(struct msg_queue_header))

struct msg_queue
{
	struct msg_queue_header header;
	uint8 buf[QUEUE_LEN];
};

struct queue_descr
{
	struct msg_queue *send;
	struct msg_queue *recv;
};

int ipc_connect(pid_t p1, pid_t p2);
int ipc_send(struct queue_descr *queue, uint8 *ptr, uint16 size);
int ipc_receive(struct queue_descr *queue, uint8 *ptr, uint16 size);

struct msg_queue *queue_create(pid_t owner);
#endif
