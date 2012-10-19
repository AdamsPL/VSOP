#include "drivers.h"
#include "util.h"
#include "process.h"
#include "interrupts.h"
#include "message.h"

struct process *handlers[256];

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
	struct message *msg;

	if (!handlers[irq])
		return INT_ERROR;

	msg = message_alloc(sizeof(buf), (uint8*)buf);

	proc_send(msg, handlers[irq]);
	return INT_OK;
}

void driver_register(pid_t pid, int irq)
{
	struct process *proc = proc_get_by_pid(pid);
	handlers[irq] = proc;
	interrupts_register_handler(irq, driver_irq_notify);
}
