#ifndef _STREAM_H
#define _STREAM_H

#include "config.h"

#define STREAM_BUF_LEN 1024
#define MAX_DESCRIPTORS 32

struct thread;

struct stream
{
	uint16 read;
	uint16 write;
	uint8 buf[STREAM_BUF_LEN];
};

struct iostream
{
	struct stream *input;
	struct stream *output;
};

struct iostream_descriptors
{
	struct iostream iostreams[MAX_DESCRIPTORS];
};

void stream_write(struct stream *this, uint8 *buf, uint32 len);
void stream_read(struct stream *this, uint8 *buf, uint32 len);
uint32 stream_read_length(struct stream *this);
uint32 stream_write_length(struct stream *this);
struct stream *stream_new();

int iostream_select(struct iostream_descriptors *descr);
int iostream_attach(struct iostream_descriptors *descr, struct stream *input, struct stream *output);

uint8 stream_select_event(struct thread *this);
uint8 stream_read_event(struct thread *this);
uint8 stream_write_event(struct thread *this);

#endif


