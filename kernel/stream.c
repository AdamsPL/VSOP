#include "stream.h"

#include "scheduler.h"
#include "memory.h"

static inline uint16 next_step(uint16 ptr)
{
	return (ptr + 1) % STREAM_BUF_LEN;
}

void stream_write(struct stream *this, uint8 *buf, uint32 len)
{

	while(len-- > 0)
	{
		while(stream_write_length(this) == 0)
		{
			struct thread *cur = sched_cur_thread();
			cur->descr = this;
			sched_thread_wait(cur, stream_write_event);
		}
		this->buf[this->write] = *buf;
		this->write = next_step(this->write);
		buf++;
	}
}

void stream_read(struct stream *this, uint8 *buf, uint32 len)
{
	while(len-- > 0)
	{
		while(stream_read_length(this) == 0)
		{
			struct thread *cur = sched_cur_thread();
			cur->descr = this;
			sched_thread_wait(cur, stream_read_event);
		}
		this->read = next_step(this->read);
		*buf = this->buf[this->read];
		buf++;
	}
}

uint32 stream_write_length(struct stream *this)
{
	int result = (this->read) - (this->write) - 1;
	if (result < 0)
		result += STREAM_BUF_LEN;
	return result;
}

uint32 stream_read_length(struct stream *this)
{
	int result = (this->write) - (this->read) - 1;
	if (result < 0)
		result += STREAM_BUF_LEN;
	return result;
}

struct stream *stream_new()
{
	struct stream *result = NEW(struct stream);

	result->read = 0;
	result->write = 1;

	return result;
}

int iostream_select(struct iostream_descriptors *descr)
{
	int i;
	for (i = 0; i < MAX_DESCRIPTORS; ++i)
	{
		if (descr->iostreams[i].input == 0)
			return -1;
		if (stream_read_length(descr->iostreams[i].input) > 0)
			return i;
	}
	return -1;
}

int iostream_attach(struct iostream_descriptors *descr, struct stream *input, struct stream *output)
{
	int i;
	for (i = 0; i < MAX_DESCRIPTORS; ++i)
	{
		if (descr->iostreams[i].input != 0)
			continue;
		descr->iostreams[i].input = input;
		descr->iostreams[i].output = output;
		return i;
	}
	return -1;
}

uint8 stream_select_event(struct thread *this)
{
	return (iostream_select(&this->parent->iodescr) != -1);
}

uint8 stream_read_event(struct thread *this)
{
	if (this->descr == 0)
		return 0;
	return (stream_read_length(this->descr) > 0);
}

uint8 stream_write_event(struct thread *this)
{
	if (this->descr == 0)
		return 0;
	return (stream_write_length(this->descr) > 0);
}
