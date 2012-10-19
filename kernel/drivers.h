#ifndef _DRIVERS_H
#define _DRIVERS_H

#include "config.h"

void drivers_init(void);
void driver_register(pid_t pid, int irq);

#endif 


