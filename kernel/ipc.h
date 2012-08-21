#ifndef _IPC_H
#define _IPC_H

#include "config.h"

struct msg_queue_header
{
	uint16 read;
	uint16 write;
};

/*#define QUEUE_LEN (PAGE_SIZE - sizeof(struct msg_queue_header))*/
#define QUEUE_LEN 20

struct msg_queue
{
	struct msg_queue_header header;
	uint8 buf[QUEUE_LEN];
};

struct queue_descr
{
	queue_id send_to;
	queue_id rec_from;
	uint8 in_use;
};

uint8 ipc_connect(pid_t p1, pid_t p2);
int ipc_send(queue_id id, uint8 *ptr, uint16 size);
int ipc_receive(queue_id id, uint8 *ptr, uint16 size);
int ipc_empty(queue_id id);

queue_id queue_create();
#endif
