#include "drivers.h"
#include "util.h"
#include "ipc.h"
#include "multitasking.h"

struct driver drivers[128];
int handlers[128];

void drivers_init()
{
	int i;
	for (i = 0; i < 128; ++i)
		handlers[i] = -1;
}

int server_get(char *name)
{
	int i = 0;
	while(kstrcmp(name, drivers[i].name)){
		++i;
		if (i == 128)
			return -1;
	}
	return drivers[i].pid;
}

int server_set(pid_t pid, char *name, int irq)
{
	int i = 0;
	int descr;

	/*
	while(drivers[i].name[0] != '\0')
		++i;
	drivers[i].pid = pid;
	kstrncpy((uint8*)drivers[i].name, (uint8*)name, 24);
	if (irq == -1)
		return -1;
	handlers[irq] = queue_create();
	descr = proc_map_queue(pid, 0, handlers[irq]);
	*/
	return descr;
}

void server_irq_notify(int irq)
{
	if (handlers[irq] == -1)
		return;
	ipc_send(handlers[irq], (uint8*)"irq!", 4);
}
