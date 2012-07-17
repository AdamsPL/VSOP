#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "config.h"

struct Scheduler;

void sched_init_all();
void sched_tick(struct Scheduler *this);
struct Scheduler *sched_current();

#endif 
