#include "drivers.h"
#include "util.h"
#include "interrupts.h"
#include "stream.h"

struct stream *handlers[256];

void drivers_init()
{
	int i;
	for (i = 0; i < 256; ++i)
		handlers[i] = 0;
}

uint8 driver_irq_notify(struct thread_state *state)
{
	int irq = state->int_id;
	char buf[] = "int";

	if (!handlers[irq])
		return INT_ERROR;

	stream_write(handlers[irq], (uint8*)buf, sizeof(buf));

	return INT_OK;
}

int driver_register(struct process *proc, int irq)
{
	handlers[irq] = stream_new();
	return iostream_attach(&proc->iodescr, handlers[irq], handlers[irq]);
	interrupts_register_handler(irq, driver_irq_notify);
}
