#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "config.h"
#include "thread.h"

void sched_init();
void sched_thread_ready(struct thread *thread);
void sched_thread_sleep(uint64 ticks);
pid_t sched_cur_proc();

#endif 
