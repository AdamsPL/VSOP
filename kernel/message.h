#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "config.h"

struct message
{
	uint32 length;
	uint8 *buf;
};

struct message *message_alloc(uint32 length, uint8 *buf);
void message_free(struct message *this);

#endif
