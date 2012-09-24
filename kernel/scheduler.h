#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "config.h"
#include "thread.h"

void sched_init();
void sched_thread_ready(struct thread *thread);
struct thread *sched_current_thread();

#endif 
