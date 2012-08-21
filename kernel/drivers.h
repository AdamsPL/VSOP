#ifndef _DRIVERS_H
#define _DRIVERS_H

#include "config.h"

struct driver
{
	char name[32];
	pid_t pid;
};


void drivers_init();

int server_get(char *name);
int server_set(pid_t pid, char *name, int irq);
void server_irq_notify(int irq);

#endif 


