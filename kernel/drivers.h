#ifndef _DRIVERS_H
#define _DRIVERS_H

#include "config.h"
#include "process.h"

void drivers_init(void);
int driver_register(struct process *proc, int irq);

#endif 


