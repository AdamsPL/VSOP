#include "message.h"
#include "memory.h"

struct message *message_alloc(uint32 length, uint8 *buf)
{
	struct message *result = NEW(struct message);

	result->length = length;
	result->buf = kmalloc(length);
	kmemcpy(result->buf, buf, length);

	return result;
}

void message_free(struct message *this)
{
	kfree(this->buf, this->length);
	DELETE(this);
}
